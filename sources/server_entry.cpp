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
	m_challengeRecv = std::atoll(data["challenge"].value().c_str());
	m_protocol = std::atoll(data["protocol"].value().c_str());
	m_players = std::atoll(data["players"].value().c_str());
	m_maxPlayers = std::atoll(data["max"].value().c_str());
	m_bots = std::atoll(data["bots"].value().c_str());
	m_regionCode = std::atoll(data["region"].value().c_str());

	m_gamedir = data["gamedir"].value();
	m_mapName = data["map"].value();
	m_clientVersion = data["version"].value();
	m_osType = data["os"].value();
	m_product = data["product"].value();
	m_dedicated = !data["type"].value().compare("d") ? true : false;
	m_passwordUsed = !data["password"].value().compare("0") ? false : true;
	m_secured = !data["secure"].value().compare("0") ? false : true;
	m_lanMode = !data["lan"].value().compare("0") ? false : true;
	m_natBypass = !data["nat"].value().compare("0") ? false : true;
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
