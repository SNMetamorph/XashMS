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
#include "binary_output_stream.h"
#include "net_address.h"
#include "server_list.h"
#include <vector>
#include <string>
#include <optional>
#include <stdint.h>

class ClientQueryResponse
{
public:
	static constexpr const char *Header = "\xff\xff\xff\xff" "f\n";

	ClientQueryResponse(bool natBypass,
		std::optional<uint32_t> queryKey,
		std::optional<uint32_t> clientProtocol,
		const NetAddress &clientAddr,
		const ServerList::EntryContainer &servers, 
		const std::string &gamedir);

	void Serialize(BinaryOutputStream &stream, std::vector<NetAddress> &natServers) const;

private:
	bool m_natBypassMode;
	NetAddress m_clientAddress;
	std::string m_gamedir;
	std::optional<uint32_t> m_queryKey;
	std::optional<uint32_t> m_clientProtocol;
	const ServerList::EntryContainer &m_servers;
};
