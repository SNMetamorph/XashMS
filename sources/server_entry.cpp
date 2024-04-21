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

#include "server_entry.h"
#include <event2/util.h>

ServerEntry::ServerEntry(const NetAddress &address) :
	m_address(address),
	m_bots(0),
	m_players(0),
	m_protocol(0),
	m_regionCode(0),
	m_challengeRecv(0),
	m_dedicated(false),
	m_lanMode(false),
	m_maxPlayers(0),
	m_natBypass(false),
	m_passwordUsed(false),
	m_secured(false),
	m_approved(false)
{
	m_keepAliveTimer.Reset();
	m_keepAliveTimer.SetInterval(5.0);
	m_challengeSendTimer.SetInterval(5.0);
	evutil_secure_rng_get_bytes(&m_challenge, sizeof(m_challenge));
}

void ServerEntry::Update(InfostringData &data)
{
	m_challengeRecv = std::atoll(data.Get("challenge").c_str());
	m_protocol = std::atoll(data.Get("protocol").c_str());
	m_players = std::atoll(data.Get("players").c_str());
	m_maxPlayers = std::atoll(data.Get("max").c_str());
	m_bots = std::atoll(data.Get("bots").c_str());
	m_regionCode = std::atoll(data.Get("region").c_str());

	m_gamedir = data.Get("gamedir");
	m_mapName = data.Get("map");
	m_clientVersion = data.Get("version");
	m_osType = data.Get("os");
	m_product = data.Get("product");
	m_dedicated = data.Get("type").compare("d") == 0 ? true : false;
	m_passwordUsed = data.Get("password").compare("0") == 0 ? false : true;
	m_secured = data.Get("secure").compare("0") == 0 ? false : true;
	m_lanMode = data.Get("lan").compare("0") == 0 ? false : true;
	m_natBypass = data.Get("nat").compare("0") == 0 ? false : true;
}

bool ServerEntry::ChallengeDelay() const
{
	return !m_challengeSendTimer.CycleElapsed();
}

void ServerEntry::ResetChallengeDelay()
{
	m_challengeSendTimer.Reset();
}

void ServerEntry::ResetTimeout()
{
	m_keepAliveTimer.Reset();
	m_keepAliveTimer.SetInterval(600.0);
}

void ServerEntry::Approve()
{
	m_approved = true;
}

bool ServerEntry::ValidateChallenge() const
{
	return m_challenge == m_challengeRecv;
}

bool ServerEntry::TimedOut() const
{
	return m_keepAliveTimer.CycleElapsed();
}
