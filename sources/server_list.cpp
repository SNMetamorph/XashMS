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

#include "server_list.h"
#include <event2/util.h>
#include <algorithm>

ServerList::ServerList(ConfigManager &configManager) : 
	m_configManager(configManager)
{
}

void ServerList::UpdateState()
{
	CleanupStallServers();
	RemoveTimeoutChallenges();
}

ServerEntry &ServerList::Insert(const NetAddress &address)
{
	if (m_serverCountMap.count(address) < 1)
	{
		m_serversMap.insert({ address, std::move(ServerEntry(address)) });
		m_serverCountMap[address] += 1;
	}
	return m_serversMap.at(address);
}

bool ServerList::Contains(const NetAddress &addr) const
{
	return m_serversMap.count(addr) > 0;
}

uint32_t ServerList::GenerateChallenge(const NetAddress &address)
{
	if (m_challengeMap.count(address) < 1)
	{
		uint32_t challenge;
		evutil_secure_rng_get_bytes(&challenge, sizeof(challenge));
		m_challengeMap.insert({ address, ChallengeEntry(challenge) });
	}
	return m_challengeMap.at(address).GetValue();
}

bool ServerList::CheckForChallenge(const NetAddress &address) const
{
	return m_challengeMap.count(address) > 0;
}

bool ServerList::ValidateChallenge(const NetAddress &address, uint32_t challenge) const
{
	if (m_challengeMap.count(address) < 1) {
		return false;
	}
	return m_challengeMap.at(address).GetValue() == challenge;
}

size_t ServerList::GetCountForAddress(const NetAddress &addr) const
{
	return (m_serverCountMap.count(addr) < 1) ? 0 : m_serverCountMap.at(addr);
}

void ServerList::Remove(const NetAddress &address)
{
	m_serverCountMap[address] -= 1;
	if (m_serverCountMap[address] == 0) {
		m_serverCountMap.erase(address);
	}
	m_challengeMap.erase(address);
	m_serversMap.erase(address);
}

void ServerList::CleanupStallServers()
{
	for (auto it = m_serversMap.begin(); it != m_serversMap.end();)
	{
		const auto &entry = it->second;
		if (entry.Timeout(m_configManager.GetData().GetServerTimeoutInterval())) 
		{
			auto address = it->first;
			it = it++; // increment iterator before it will be invalidated
			Remove(address);
		}
		else {
			it++;
		}
	}
	// TODO operations to clean up memory that was used for removed elements
}

void ServerList::RemoveTimeoutChallenges()
{
	for (auto it = m_challengeMap.begin(); it != m_challengeMap.end();)
	{
		const auto &entry = it->second;
		if (entry.Timeout(m_configManager.GetData().GetChallengeTimeoutInterval())) {
			it = m_challengeMap.erase(it);
		}
		else {
			it++;
		}
	}
}
