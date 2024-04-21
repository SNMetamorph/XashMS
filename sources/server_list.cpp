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
#include <algorithm>

void ServerList::CleanupStallServers()
{
	auto predicate = [](const ServerEntry &entry) {
		return entry.TimedOut();
	};
	auto it = std::remove_if(m_entries.begin(), m_entries.end(), predicate);
	m_entries.erase(it, m_entries.end());
}

void ServerList::Insert(const NetAddress &address)
{
	m_entries.emplace_back(address);
}

bool ServerList::Contains(const NetAddress &addr) const
{
	auto predicate = [&addr](const ServerEntry &entry) {
		return addr.Equals(entry.GetAddress(), true);
	};
	return std::find_if(m_entries.begin(), m_entries.end(), predicate) != std::end(m_entries);
}

ServerEntry *ServerList::FindEntry(const NetAddress &address)
{
	auto predicate = [&address](const ServerEntry &entry) {
		return address.Equals(entry.GetAddress(), true);
	};
	auto iter = std::find_if(m_entries.begin(), m_entries.end(), predicate);
	return iter != std::end(m_entries) ? &(*iter) : nullptr;
}

size_t ServerList::CountForAddress(const NetAddress &addr) const
{
	auto predicate = [&addr](const ServerEntry &entry) {
		return addr.Equals(entry.GetAddress(), false);
	};
	return std::count_if(m_entries.begin(), m_entries.end(), predicate);
}
