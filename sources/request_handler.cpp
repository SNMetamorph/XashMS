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

	if (std::memcmp(recvBuffer.data(), MasterProtocol::clientQuery, 1) == 0) {
		ProcessClientQuery(socket, sourceAddr);
	}
	else if (std::memcmp(recvBuffer.data(), MasterProtocol::challengeRequest, 2) == 0) {
		ProcessChallengeRequest(socket, sourceAddr);
	}
	else if (std::memcmp(recvBuffer.data(), MasterProtocol::addServer, 2) == 0) {
		ProcessAddServerRequest(socket, sourceAddr);
	}
}

void RequestHandler::ProcessClientQuery(Socket &socket, const NetAddress &sourceAddr)
{
	auto &recvBuffer = socket.GetDataBuffer();
	BinaryInputStream request(recvBuffer.data(), recvBuffer.size());

	request.SkipBytes(2); // skip header
	if (request.EndOfFile()) {
		return; // invalid request
	}

	// placeholder addr, ignored
	if (!request.SkipString()) { 
		return; 
	}

	std::string queryInfo;
	if (!request.ReadString(queryInfo)) {
		return; 
	}

	size_t slashPos = queryInfo.find_first_of('\\');
	if (slashPos == std::string::npos) {
		return; // invalid query info string
	}

	InfostringData data(queryInfo);
	if (!ValidateClientQueryInfostring(data)) {
		return; // invalid query info, ignore request
	}

	if (!data["clver"].has_value()) {
		SendFakeServerInfo(socket, sourceAddr, data);
	}
	else {
		SendClientQueryResponse(socket, sourceAddr, data);
	}

	Utils::Log("Client query: {}:{}, gamedir={}, clver={}, nat={}\n", 
		sourceAddr.ToString(), 
		sourceAddr.GetPort(), 
		data["gamedir"].value(),
		data["clver"].value(),
		data["nat"].value());
}

void RequestHandler::ProcessChallengeRequest(Socket &socket, const NetAddress &sourceAddr)
{
	if (m_serverList.GetCountForAddress(sourceAddr) >= ServerList::GetQuotaPerAddress()) {
		return; // too much servers for this IP address
	}

	if (m_serverList.CheckForChallenge(sourceAddr)) {
		return; // this server already got challenge
	}

	auto &recvBuffer = socket.GetDataBuffer();
	BinaryInputStream request(recvBuffer.data(), recvBuffer.size());
	std::optional<uint32_t> clientChallenge = std::nullopt;

	if (recvBuffer.size() == 6)
	{
		// write for second challenge 
		request.SkipBytes(2);
		clientChallenge = request.Read<uint32_t>();
	}

	uint32_t challenge = m_serverList.GenerateChallenge(sourceAddr);
	SendChallengeResponse(socket, sourceAddr, challenge, clientChallenge);
}

void RequestHandler::ProcessAddServerRequest(Socket &socket, const NetAddress &sourceAddr)
{
	auto &recvBuffer = socket.GetDataBuffer();
	BinaryInputStream request(recvBuffer.data(), recvBuffer.size());

	request.SkipBytes(2); // skip header bytes
	if (request.EndOfFile()) {
		return; // invalid request length
	}

	if (!m_serverList.CheckForChallenge(sourceAddr)) 
	{
		Utils::Log("Server skipped challenge request: {}:{}\n", sourceAddr.ToString(), sourceAddr.GetPort());
		return;
	}

	std::string infostring;
	InfostringData data;
	request.ReadString(infostring);
	data.Parse(infostring);

	// check for challenge correctness
	if (!data["challenge"].has_value()) {
		return; // missing challenge number in infostring
	}
	else
	{
		uint32_t challengeRecv = std::atoll(data["challenge"].value().c_str());
		if (!m_serverList.ValidateChallenge(sourceAddr, challengeRecv))
		{
			Utils::Log("Incorrect challenge from {}:{}: value {}\n", sourceAddr.ToString(), sourceAddr.GetPort(), challengeRecv);
			return;
		}
	}

	// check for request infostring correctness and fullness
	if (!ValidateAddServerInfostring(data))
	{
		Utils::Log("Invalid add server query infostring: {}:{}\n", sourceAddr.ToString(), sourceAddr.GetPort());
		return;
	}

	bool serverExists = m_serverList.Contains(sourceAddr);
	ServerEntry &server = m_serverList.Insert(sourceAddr);
	server.Update(data); 
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

void RequestHandler::SendClientQueryResponse(Socket &socket, const NetAddress &clientAddr, InfostringData &data)
{
	std::vector<uint8_t> buffer;
	BinaryOutputStream response(buffer);

	response.WriteString(MasterProtocol::queryPacketHeader);
	if (data["key"].has_value())
	{
		try {
			uint32_t queryKey = std::stoi(data["key"].value(), nullptr, 16);
			response.WriteByte(0x7F);
			response.Write<uint32_t>(queryKey);
			response.WriteByte(0x00);
		}
		catch (const std::exception &ex) {
			return; // invalid data in query key
		}
	}

	bool natBypass = data["nat"].value().compare("0") != 0;
	for (const auto &[serverAddr, entry] : m_serverList.GetEntriesCollection())
	{
		if (serverAddr.GetAddressFamily() != clientAddr.GetAddressFamily())
			continue;

		if (entry.NatBypassEnabled() != natBypass)
			continue;

		if (data["gamedir"].value().compare(entry.GetGamedir()) != 0)
			continue;

		if (data["protocol"].has_value())
		{
			uint32_t clientProtocol = std::atoi(data["protocol"].value().c_str());
			if (entry.GetProtocolVersion() != clientProtocol) {
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

void RequestHandler::SendFakeServerInfo(Socket &socket, const NetAddress &dest, InfostringData &infostringData)
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
		infostring.Insert("gamedir", infostringData["gamedir"].value());

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

bool RequestHandler::ValidateClientQueryInfostring(const InfostringData &data)
{
	if (!data["gamedir"].has_value()) {
		return false;
	}
	if (!data["nat"].has_value()) {
		return false;
	}
	return true;
}

bool RequestHandler::ValidateAddServerInfostring(const InfostringData &data)
{
	if (!data["protocol"].has_value()) {
		return false;
	}
	if (!data["players"].has_value()) {
		return false;
	}
	if (!data["max"].has_value()) {
		return false;
	}
	if (!data["bots"].has_value()) {
		return false;
	}
	if (!data["region"].has_value()) {
		return false;
	}
	if (!data["gamedir"].has_value()) {
		return false;
	}
	if (!data["map"].has_value()) {
		return false;
	}
	if (!data["version"].has_value()) {
		return false;
	}
	if (!data["os"].has_value()) {
		return false;
	}
	if (!data["product"].has_value()) {
		return false;
	}
	if (!data["type"].has_value()) {
		return false;
	}
	if (!data["password"].has_value()) {
		return false;
	}
	if (!data["secure"].has_value()) {
		return false;
	}
	if (!data["lan"].has_value()) {
		return false;
	}
	if (!data["nat"].has_value()) {
		return false;
	}
	return true;
}
