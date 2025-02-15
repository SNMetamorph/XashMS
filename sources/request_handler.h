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

#pragma once
#include "timer.h"
#include "socket.h"
#include "net_address.h"
#include "config_manager.h"
#include "server_list.h"
#include "admin_command_handler.h"
#include "client_query_request.h"
#include "server_challenge_request.h"
#include "server_append_request.h"
#include "admin_command_request.h"
#include <vector>
#include <optional>
#include <unordered_set>
#include <unordered_map>
#include <string>

class RequestHandler
{
public:
	RequestHandler(ServerList &serverList, ConfigManager &configManager);
	void UpdateState();
	void HandlePacket(Socket &socket, const NetAddress &sourceAddr);

private:
	void HandleRequest(Socket &socket, const NetAddress &sourceAddr);

	void ProcessClientQuery(Socket &socket, const NetAddress &sourceAddr, ClientQueryRequest &req);
	void ProcessChallengeRequest(Socket &socket, const NetAddress &sourceAddr, ServerChallengeRequest &req);
	void ProcessAddServerRequest(Socket &socket, const NetAddress &sourceAddr, ServerAppendRequest &req);
	void ProcessAdminChallengeRequest(Socket &socket, const NetAddress &sourceAddr);
	void ProcessAdminCommandRequest(const NetAddress &sourceAddr, AdminCommandRequest &req);

	void SendClientQueryResponse(Socket &socket, const NetAddress &clientAddr, ClientQueryRequest &req);
	void SendChallengeResponse(Socket &socket, const NetAddress &dest, uint32_t ch1, std::optional<uint32_t> ch2);
	void SendFakeServerInfo(Socket &socket, const NetAddress &dest, const std::string &gamedir);
	void SendNatAnnouncements(Socket &socket, const NetAddress &clientAddr);

	ServerList &m_serverList;
	ConfigManager &m_configManager;
	AdminCommandHandler m_adminCommandHandler;
	std::vector<NetAddress> m_natAnnouncedServers;
	std::unordered_set<NetAddress, NetAddressHash> m_banlist;
	std::unordered_map<NetAddress, uint32_t, NetAddressHash> m_packetRateMap;
};
