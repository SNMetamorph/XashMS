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
#include "net_address.h"
#include "server_entry.h"
#include "config_manager.h"
#include <string>
#include <vector>
#include <unordered_map>
#include <stdint.h>

class ServerList
{
public:
	using EntryContainer = std::unordered_map<NetAddress, ServerEntry, NetAddressPortHash>;
	struct AdminChallenge
	{
		uint32_t master;
		uint32_t hash;
	};

	ServerList(ConfigManager &configManager);
	void UpdateState();
	ServerEntry &Insert(const NetAddress &address);
	bool Contains(const NetAddress &address) const;
	void BanAddress(const NetAddress &address);

	uint32_t GenerateChallenge(const NetAddress &address);
	bool CheckForChallenge(const NetAddress &address) const;
	bool ValidateChallenge(const NetAddress &address, uint32_t challenge) const;

	AdminChallenge GetAdminChallenge(const NetAddress &address);
	bool CheckAdminChallenge(const NetAddress &address) const;

	size_t GetCountForAddress(const NetAddress &addr) const;
	const EntryContainer &GetEntriesCollection() const { return m_serversMap; }

private:
	void Remove(const NetAddress &address);
	void RemoveExpiredServers();
	void RemoveExpiredChallenges();
	void RemoveExpiredAdminChallenges();

	ConfigManager& m_configManager;
	EntryContainer m_serversMap;
	std::unordered_map<NetAddress, int32_t, NetAddressHash> m_serverCountMap;
	std::unordered_map<NetAddress, Expirable<uint32_t>, NetAddressPortHash> m_challengeMap;
	std::unordered_map<NetAddress, Expirable<AdminChallenge>, NetAddressPortHash> m_adminChallengeMap;
};
