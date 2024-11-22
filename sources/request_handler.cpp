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
#include "admin_challenge_request.h"
#include "admin_challenge_response.h"
#include "server_challenge_response.h"
#include "server_nat_announce.h"
#include "client_query_response.h"
#include "utils.h"

RequestHandler::RequestHandler(ServerList &serverList, ConfigManager &configManager) :
	m_serverList(serverList),
	m_configManager(configManager),
	m_adminCommandHandler(serverList, configManager, m_banlist)
{
}

void RequestHandler::UpdateState()
{
	m_packetRateMap.clear();
	for (auto it = m_rateLimitBanlist.begin(); it != m_rateLimitBanlist.end(); it++)
	{
		const auto &entry = it->second;
		if (entry.CycleElapsed()) {
			it = m_rateLimitBanlist.erase(it);
		}
		else {
			it++;
		}
	}
}

void RequestHandler::HandlePacket(Socket &socket, const NetAddress &sourceAddr)
{
	if (m_banlist.count(sourceAddr) > 0 || m_rateLimitBanlist.count(sourceAddr) > 0) {
		return; // ignore packets from banned addresses
	}
	else
	{
		m_packetRateMap[sourceAddr] += 1;
		if (m_packetRateMap[sourceAddr] > m_configManager.GetData().GetPacketRateLimit()) 
		{
			m_rateLimitBanlist.insert({ sourceAddr, Timer(m_configManager.GetData().GetRateLimitBanTime()) });
			Utils::Log("Address {} banned due to exceeding packet rate limit\n", sourceAddr.ToString());
		}
	}

	auto &recvBuffer = socket.GetDataBuffer();
	if (recvBuffer.size() < 2) {
		return; // invalid size packet, ignore it
	}
	HandleRequest(socket, sourceAddr);
}

void RequestHandler::HandleRequest(Socket &socket, const NetAddress &sourceAddr)
{
	auto &recvBuffer = socket.GetDataBuffer();
	BinaryInputStream stream(recvBuffer.data(), recvBuffer.size());
	if (std::memcmp(recvBuffer.data(), ClientQueryRequest::Header, 1) == 0)
	{
		auto request = ClientQueryRequest::Parse(stream);
		if (request.has_value()) {
			ProcessClientQuery(socket, sourceAddr, request.value());
		}
	}
	else if (std::memcmp(recvBuffer.data(), ServerChallengeRequest::Header, 2) == 0) 
	{
		if (m_serverList.GetCountForAddress(sourceAddr) >= m_configManager.GetData().GetServerCountQuota()) {
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
	else if (std::memcmp(recvBuffer.data(), ServerAppendRequest::Header, 2) == 0)
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
	else if (std::memcmp(recvBuffer.data(), AdminChallengeRequest::Header, 14) == 0) 
	{
		ProcessAdminChallengeRequest(socket, sourceAddr);
	}
	else if (std::memcmp(recvBuffer.data(), AdminCommandRequest::Header, 5) == 0) 
	{
		if (!m_serverList.CheckAdminChallenge(sourceAddr)) {
			return;
		}

		auto request = AdminCommandRequest::Parse(stream, m_configManager.GetData().GetAdminHashLength());
		if (request.has_value()) {
			ProcessAdminCommandRequest(sourceAddr, request.value());
		}
	}
}

void RequestHandler::ProcessClientQuery(Socket &socket, const NetAddress &sourceAddr, ClientQueryRequest &request)
{
	if (request.ClientOutdated()) {
		SendFakeServerInfo(socket, sourceAddr, request.GetGamedir());
	}
	else 
	{
		SendClientQueryResponse(socket, sourceAddr, request);
		SendNatAnnouncements(socket, sourceAddr);
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
	uint32_t challengeRecv = request.GetMasterChallenge();
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

void RequestHandler::ProcessAdminChallengeRequest(Socket &socket, const NetAddress &sourceAddr)
{
	if (m_serverList.CheckAdminChallenge(sourceAddr)) {
		return; 
	}

	uint8_t buffer[64];
	BinaryOutputStream stream(buffer, sizeof(buffer));
	AdminChallenge challenge = m_serverList.GetAdminChallenge(sourceAddr);

	AdminChallengeResponse response(challenge.master, challenge.hash);
	response.Serialize(stream);
	socket.SendTo(sourceAddr, buffer, sizeof(buffer));
}

void RequestHandler::ProcessAdminCommandRequest(const NetAddress &sourceAddr, AdminCommandRequest &request)
{
	auto challenge = m_serverList.GetAdminChallenge(sourceAddr);
	if (challenge.master != request.GetMasterChallenge())
	{
		Utils::Log("Incorrect admin challenge from {}:{}\n", sourceAddr.ToString(), sourceAddr.GetPort());
		return;
	}
	m_adminCommandHandler.HandleCommandRequest(sourceAddr, request, challenge);
}

void RequestHandler::SendClientQueryResponse(Socket &socket, const NetAddress &clientAddr, ClientQueryRequest &request)
{
	std::vector<uint8_t> buffer;
	BinaryOutputStream stream(buffer);

	auto queryKey = request.GetQueryKey();
	ClientQueryResponse response(
		request.ClientBypassingNat(), 
		queryKey, 
		request.GetProtocolVersion(), 
		clientAddr, 
		m_serverList.GetEntriesCollection(), 
		request.GetGamedir());

	response.Serialize(stream, m_natAnnouncedServers);
	socket.SendTo(clientAddr, buffer);
}

void RequestHandler::SendChallengeResponse(Socket &socket, const NetAddress &dest, uint32_t ch1, std::optional<uint32_t> ch2)
{
	uint8_t buffer[64];
	BinaryOutputStream stream(buffer, sizeof(buffer));

	ServerChallengeResponse response(ch1, ch2);
	response.Serialize(stream);
	socket.SendTo(dest, buffer, sizeof(buffer));
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

void RequestHandler::SendNatAnnouncements(Socket &socket, const NetAddress &clientAddr)
{
	uint8_t buffer[64];
	for (const auto &serverAddr : m_natAnnouncedServers) 
	{
		BinaryOutputStream stream(buffer, sizeof(buffer));
		ServerNatAnnounce response(clientAddr);
		response.Serialize(stream);
		socket.SendTo(serverAddr, stream.GetBuffer(), stream.GetLength());
	}
}
