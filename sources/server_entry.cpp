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

#include <scn/scan.h>
#include "server_entry.h"

ServerEntry::ServerEntry(const NetAddress &address) :
	m_address(address),
	m_bots(0),
	m_players(0),
	m_protocol(0),
	m_regionCode(0),
	m_dedicated(false),
	m_lanMode(false),
	m_maxPlayers(0),
	m_natBypass(false),
	m_passwordUsed(false),
	m_secured(false)
{
	m_keepAliveTimer.Reset();
}

void ServerEntry::Update(const InfostringData &data)
{
	m_protocol = scn::scan_int<uint32_t>(data["protocol"].value())->value();
	m_players = scn::scan_int<uint32_t>(data["players"].value())->value();
	m_maxPlayers = scn::scan_int<uint32_t>(data["max"].value())->value();
	m_bots = scn::scan_int<uint32_t>(data["bots"].value())->value();
	m_regionCode = scn::scan_int<uint32_t>(data["region"].value())->value();

	m_gamedir = data["gamedir"].value();
	m_mapName = data["map"].value();
	m_version = data["version"].value();
	m_osType = data["os"].value();
	m_product = data["product"].value();
	m_dedicated = data["type"].value().compare("d") ? false : true;
	m_passwordUsed = data["password"].value().compare("0") ? true : false;
	m_secured = data["secure"].value().compare("0") ? true : false;
	m_lanMode = data["lan"].value().compare("0") ? true : false;
	m_natBypass = data["nat"].value().compare("0") ? true : false;
}

void ServerEntry::ResetTimeout()
{
	m_keepAliveTimer.Reset();
}

bool ServerEntry::Expired(double interval) const
{
	return m_keepAliveTimer.IntervalElapsed(interval);
}
