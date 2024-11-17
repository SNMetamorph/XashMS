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

#include "admin_command_handler.h"
#include <cryptopp/blake2.h>
#include "utils.h"

AdminCommandHandler::AdminCommandHandler(ServerList &serverList, 
	ConfigManager &configManager, std::unordered_set<NetAddress, NetAddressHash> &banlist) :
	m_serverList(serverList),
	m_configManager(configManager),
	m_banlist(banlist)
{
}

void AdminCommandHandler::HandleCommandRequest(const NetAddress &sourceAddr, AdminCommandRequest &request, AdminChallenge &challenge)
{
	const std::string &key = m_configManager.GetData().GetAdminHashKey();
	const std::string &personal = m_configManager.GetData().GetAdminHashPersonal();
	size_t hashLength = m_configManager.GetData().GetAdminHashLength();

	CryptoPP::BLAKE2b initialHashState(
		reinterpret_cast<const uint8_t*>(key.c_str()),
		key.length(),
		nullptr,
		0,
		reinterpret_cast<const uint8_t*>(personal.c_str()),
		personal.length(),
		false,
		static_cast<uint32_t>(hashLength)
	);

	std::vector<uint8_t> digestBuffer(hashLength, '\0');
	const auto admins = m_configManager.GetData().GetAdmins();
	for (auto it = admins.begin(); it != admins.end(); it++)
	{
		const auto entry = *it;
		const size_t passwordLength = entry.password.length();
		const uint8_t *passwordData = reinterpret_cast<const uint8_t*>(entry.password.c_str());

		CryptoPP::BLAKE2b hash(initialHashState);
		hash.Update(passwordData, passwordLength);
		hash.Update(reinterpret_cast<uint8_t*>(&challenge.hash), sizeof(challenge.hash));
		hash.Final(digestBuffer.data());

		if (std::memcmp(digestBuffer.data(), request.GetHash(), hashLength) == 0) 
		{
			HandleCommand(sourceAddr, entry.name, request.GetCommand());
			return;
		}
	}

	Utils::Log("Unauthorized admin command attempt from {}:{}\n", sourceAddr.ToString(), sourceAddr.GetPort());
}

void AdminCommandHandler::HandleCommand(const NetAddress &sourceAddr, const std::string &name, const std::string &command)
{
	std::vector<std::string_view> tokens = Utils::Tokenize(command, " ");
	if (tokens.size() == 2)
	{
		std::string commandName(tokens[0]);
		std::string commandArgument(tokens[1]);
		if (commandName.compare("ban") == 0)
		{
			auto address = NetAddress::Parse(commandArgument.c_str());
			if (address.has_value()) 
			{
				HandleBanCommand(sourceAddr, name, address.value());
				return;
			}
		}
		else if (commandName.compare("unban") == 0)
		{
			auto address = NetAddress::Parse(commandArgument.c_str());
			if (address.has_value()) 
			{
				HandleUnbanCommand(sourceAddr, name, address.value());
				return;
			}
		}
	}
	Utils::Log("Admin {}({}) issued unknown command \"{}\"\n", name, sourceAddr.ToString(), command);
}

void AdminCommandHandler::HandleBanCommand(const NetAddress &sourceAddr, const std::string &name, const NetAddress &targetAddr)
{
	m_banlist.insert(targetAddr);
	m_serverList.BanAddress(targetAddr);
	Utils::Log("Admin {}({}) banned address {}\n", name, sourceAddr.ToString(), targetAddr.ToString());
}

void AdminCommandHandler::HandleUnbanCommand(const NetAddress &sourceAddr, const std::string &name, const NetAddress &targetAddr)
{
	m_banlist.erase(targetAddr);
	Utils::Log("Admin {}({}) unbanned address {}\n", name, sourceAddr.ToString(), targetAddr.ToString());
}
