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
#include <string>
#include <vector>
#include <stdint.h>

class ServerList
{
public:
	void CleanupStallServers();
	void Insert(const NetAddress &address);
	bool Contains(const NetAddress &address) const;
	ServerEntry *FindEntry(const NetAddress &address);
	size_t CountForAddress(const NetAddress &addr) const;
	const std::vector<ServerEntry> &GetEntriesCollection() const { return m_entries; }
	static constexpr size_t GetMaxServersForIP() { return 14; }

private:
	std::vector<ServerEntry> m_entries;
};
