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
#include "infostring_data.h"
#include <string>
#include <vector>
#include <stdint.h>

class ServerEntry
{
public:
	ServerEntry(const NetAddress &address);
	ServerEntry(const ServerEntry&) = default;
	ServerEntry(ServerEntry&&) noexcept = default;
	ServerEntry& operator=(ServerEntry&&) = default;
	ServerEntry& operator=(const ServerEntry&) = default;

	void Update(InfostringData &data);
	void ResetTimeout();
	bool Timeout() const;

	bool NatBypassEnabled() const { return m_natBypass; }
	uint32_t GetProtocolVersion() const { return m_protocol; }
	uint32_t GetMaxPlayers() const { return m_maxPlayers; }
	uint32_t GetBotsCount() const { return m_bots; }
	uint32_t GetPlayersCount() const { return m_players; }
	const std::string &GetMapName() const { return m_mapName; }
	const std::string &GetGamedir() const { return m_gamedir; }
	const std::string &GetClientVersion() const { return m_clientVersion; }
	const NetAddress &GetAddress() const { return m_address; }

private:
	NetAddress m_address;
	uint32_t m_protocol;
	uint32_t m_players;
	uint32_t m_maxPlayers;
	uint32_t m_bots;
	uint32_t m_regionCode;
	std::string m_gamedir;
	std::string m_mapName;
	std::string m_clientVersion;
	std::string m_serverType;
	std::string m_osType;
	std::string m_product;
	bool m_passwordUsed;
	bool m_secured;
	bool m_lanMode;
	bool m_natBypass;
	bool m_dedicated;
	Timer m_keepAliveTimer;
};
