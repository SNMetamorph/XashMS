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
#include "net_address.h"
#include "server_list.h"
#include "config_manager.h"
#include "admin_challenge.h"
#include "admin_command_request.h"
#include <string>
#include <unordered_set>

class AdminCommandHandler
{
public:
	AdminCommandHandler(ServerList &serverList, 
		ConfigManager &configManager, 
		std::unordered_set<NetAddress, NetAddressHash> &banlist);

	void HandleCommandRequest(const NetAddress &sourceAddr, AdminCommandRequest &request, AdminChallenge &challenge);

private:
	void HandleCommand(const NetAddress &sourceAddr, const std::string &name, const std::string &command);
	void HandleBanCommand(const NetAddress &sourceAddr, const std::string &name, const NetAddress &targetAddr);
	void HandleUnbanCommand(const NetAddress &sourceAddr, const std::string &name, const NetAddress &targetAddr);

	ServerList &m_serverList;
	ConfigManager &m_configManager;
	std::unordered_set<NetAddress, NetAddressHash> &m_banlist;
};
