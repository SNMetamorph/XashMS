/*
Copyright (C) 2024 SNMetamorph

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.
*/

#include "request_handler.h"
#include "master_protocol.h"
#include "binary_input_stream.h"
#include "binary_output_stream.h"
#include "utils.h"

RequestHandler::RequestHandler(ServerList &serverList) :
	m_serverList(serverList)
{
}

void RequestHandler::HandlePacket(Socket &socket, const NetAddress &sourceAddr)
{
	auto &recvBuffer = socket.GetDataBuffer();
	if (recvBuffer.size() < 2) {
		return; // invalid size packet, ignore it
	}

	BinaryInputStream stream(recvBuffer.data(), recvBuffer.size());
	if (std::memcmp(recvBuffer.data(), MasterProtocol::clientQuery, 1) == 0)
	{
		auto request = ClientQueryRequest::Parse(stream);
		if (request.has_value()) {
			ProcessClientQuery(socket, sourceAddr, request.value());
		}
	}
	else if (std::memcmp(recvBuffer.data(), MasterProtocol::challengeRequest, 2) == 0) 
	{
		if (m_serverList.GetCountForAddress(sourceAddr) >= ServerList::GetQuotaPerAddress()) {
			return; // too much servers for this IP address
		}
		else if (m_serverList.CheckForChallenge(sourceAddr)) {
			return; // this server already got challenge
		}

		auto request = ServerChallengeRequest::Parse(stream);
		if (request.has_value()) {
			ProcessChallengeRequest(socket, sourceAddr, request.value());
		}
	}
	else if (std::memcmp(recvBuffer.data(), MasterProtocol::addServer, 2) == 0)
	{
		if (!m_serverList.CheckForChallenge(sourceAddr)) 
		{
			Utils::Log("Server skipped challenge request: {}:{}\n", sourceAddr.ToString(), sourceAddr.GetPort());
			return;
		}

		auto request = ServerAppendRequest::Parse(stream);
		if (request.has_value()) {
			ProcessAddServerRequest(socket, sourceAddr, request.value());
		}
	}
}

void RequestHandler::ProcessClientQuery(Socket &socket, const NetAddress &sourceAddr, ClientQueryRequest &request)
{
	if (request.ClientOutdated()) {
		SendFakeServerInfo(socket, sourceAddr, request.GetGamedir());
	}
	else {
		SendClientQueryResponse(socket, sourceAddr, request);
	}

	Utils::Log("Client query: {}:{}, gamedir={}, clver={}, nat={}\n", 
		sourceAddr.ToString(), 
		sourceAddr.GetPort(), 
		request.GetGamedir(),
		request.GetClientVersion().value_or("unknown"),
		request.ClientBypassingNat() ? 1 : 0);
}

void RequestHandler::ProcessChallengeRequest(Socket &socket, const NetAddress &sourceAddr, ServerChallengeRequest &request)
{
	std::optional<uint32_t> clientChallenge = request.GetClientChallenge();
	uint32_t challenge = m_serverList.GenerateChallenge(sourceAddr);
	SendChallengeResponse(socket, sourceAddr, challenge, clientChallenge);
}

void RequestHandler::ProcessAddServerRequest(Socket &socket, const NetAddress &sourceAddr, ServerAppendRequest &request)
{
	uint32_t challengeRecv = request.GetChallenge();
	if (!m_serverList.ValidateChallenge(sourceAddr, challengeRecv))
	{
		Utils::Log("Incorrect challenge from {}:{}: value {}\n", sourceAddr.ToString(), sourceAddr.GetPort(), challengeRecv);
		return;
	}

	bool serverExists = m_serverList.Contains(sourceAddr);
	ServerEntry &server = m_serverList.Insert(sourceAddr);
	server.Update(request.GetInfostringData()); 
	server.ResetTimeout();

	Utils::Log(serverExists ? "Updated " : "Added ");
	Utils::Log("server: {}:{}, game={}/{}, protocol={}, players={}/{}/{}, version={}\n", 
		sourceAddr.ToString(), 
		sourceAddr.GetPort(), 
		server.GetMapName(), 
		server.GetGamedir(), 
		server.GetProtocolVersion(),
		server.GetPlayersCount(),
		server.GetBotsCount(),
		server.GetMaxPlayers(),
		server.GetClientVersion());
}

void RequestHandler::SendClientQueryResponse(Socket &socket, const NetAddress &clientAddr, ClientQueryRequest &request)
{
	std::vector<uint8_t> buffer;
	BinaryOutputStream response(buffer);
	auto queryKey = request.GetQueryKey();

	response.WriteString(MasterProtocol::queryPacketHeader);
	if (queryKey.has_value())
	{
		response.WriteByte(0x7F);
		response.Write<uint32_t>(queryKey.value());
		response.WriteByte(0x00);
	}

	bool natBypass = request.ClientBypassingNat();
	auto clientProtocol = request.GetProtocolVersion();
	for (const auto &[serverAddr, entry] : m_serverList.GetEntriesCollection())
	{
		if (serverAddr.GetAddressFamily() != clientAddr.GetAddressFamily())
			continue;

		if (entry.NatBypassEnabled() != natBypass)
			continue;

		if (request.GetGamedir().compare(entry.GetGamedir()) != 0)
			continue;

		if (clientProtocol.has_value())
		{
			if (entry.GetProtocolVersion() != clientProtocol.value()) {
				continue;
			}
		}

		if (natBypass) {
			SendNatBypassNotify(socket, serverAddr, clientAddr);
		}
		response.WriteNetAddress(serverAddr);
	}

	// write null address as an end of message marker
	response.WriteByte(0x00, 6);
	socket.SendTo(clientAddr, buffer);
}

void RequestHandler::SendChallengeResponse(Socket &socket, const NetAddress &dest, uint32_t ch1, std::optional<uint32_t> ch2)
{
	std::vector<uint8_t> buffer;
	BinaryOutputStream response(buffer);
	
	response.WriteString(MasterProtocol::challengePacketHeader);
	response.Write<uint32_t>(ch1);
	if (ch2.has_value()) {
		response.Write<uint32_t>(ch2.value());
	}
	socket.SendTo(dest, buffer);
}

void RequestHandler::SendFakeServerInfo(Socket &socket, const NetAddress &dest, const std::string &gamedir)
{
	std::vector<uint8_t> data;
	BinaryOutputStream response(data);

	auto sendServerInfo = [&](std::string message) {
		InfostringData infostring;
		infostring.Insert("host", message);
		infostring.Insert("map", "update");
		infostring.Insert("dm", "0");
		infostring.Insert("team", "0");
		infostring.Insert("coop", "0");
		infostring.Insert("numcl", "32");
		infostring.Insert("maxcl", "32");
		infostring.Insert("gamedir", gamedir);

		data.clear();
		response.WriteString(MasterProtocol::fakeServerInfoHeader);
		response.WriteString(infostring.ToString().c_str());
		socket.SendTo(dest, data);
	};

	sendServerInfo(u8"This version is not");
	sendServerInfo(u8"supported anymore");
	sendServerInfo(u8"Please update Xash3DFWGS");
	sendServerInfo(u8"From GooglePlay or GitHub");
	sendServerInfo(u8"Эта версия");
	sendServerInfo(u8"устарела");
	sendServerInfo(u8"Обновите Xash3DFWGS c");
	sendServerInfo(u8"GooglePlay или GitHub");
}

void RequestHandler::SendNatBypassNotify(Socket &socket, const NetAddress &dest, const NetAddress &client)
{
	std::vector<uint8_t> tempBuffer;
	BinaryOutputStream natBypassPacket(tempBuffer);
	std::string addrString = fmt::format("{}:{}", client.ToString(), client.GetPort());

	natBypassPacket.WriteString(MasterProtocol::natBypassPacketHeader);
	natBypassPacket.WriteBytes(addrString.c_str(), addrString.size());
	socket.SendTo(dest, tempBuffer);
}
