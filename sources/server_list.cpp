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
	RemoveExpiredServers();
	RemoveExpiredChallenges();
	RemoveExpiredAdminChallenges();
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

void ServerList::BanAddress(const NetAddress &address)
{
	m_serverCountMap.erase(address);
	for (auto it = m_serversMap.begin(); it != m_serversMap.end();)
	{
		const auto &entry = it->second;
		const auto serverAddress = it->first;
		if (serverAddress.Equals(address)) {
			it = m_serversMap.erase(it);
		}
		else {
			it++;
		}
	}
}

uint32_t ServerList::GenerateChallenge(const NetAddress &address)
{
	if (m_challengeMap.count(address) < 1)
	{
		uint32_t challenge;
		evutil_secure_rng_get_bytes(&challenge, sizeof(challenge));
		m_challengeMap.insert({ address, Expirable<uint32_t>(challenge) });
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

AdminChallenge ServerList::GetAdminChallenge(const NetAddress &address)
{
	if (m_adminChallengeMap.count(address) < 1)
	{
		AdminChallenge challenge;
		evutil_secure_rng_get_bytes(&challenge.hash, sizeof(challenge.hash));
		evutil_secure_rng_get_bytes(&challenge.master, sizeof(challenge.master));
		m_adminChallengeMap.insert({ address, Expirable<AdminChallenge>(challenge) });
	}
	return m_adminChallengeMap.at(address).GetValue();
}

bool ServerList::CheckAdminChallenge(const NetAddress &address) const
{
	return m_adminChallengeMap.count(address) > 0;
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

void ServerList::RemoveExpiredServers()
{
	for (auto it = m_serversMap.begin(); it != m_serversMap.end();)
	{
		const auto &entry = it->second;
		if (entry.Expired(m_configManager.GetData().GetServerTimeoutInterval())) 
		{
			auto address = it->first;
			it = m_serversMap.erase(it); 
			Remove(address);
		}
		else {
			it++;
		}
	}
	// TODO operations to clean up memory that was used for removed elements
}

void ServerList::RemoveExpiredChallenges()
{
	for (auto it = m_challengeMap.begin(); it != m_challengeMap.end();)
	{
		const auto &entry = it->second;
		if (entry.Expired(m_configManager.GetData().GetChallengeTimeoutInterval())) {
			it = m_challengeMap.erase(it);
		}
		else {
			it++;
		}
	}
}

void ServerList::RemoveExpiredAdminChallenges()
{
	for (auto it = m_adminChallengeMap.begin(); it != m_adminChallengeMap.end();)
	{
		const auto &entry = it->second;
		if (entry.Expired(m_configManager.GetData().GetChallengeTimeoutInterval())) {
			it = m_adminChallengeMap.erase(it);
		}
		else {
			it++;
		}
	}
}
