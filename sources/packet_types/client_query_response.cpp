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

#include "client_query_response.h"

ClientQueryResponse::ClientQueryResponse(bool natBypass, 
	std::optional<uint32_t> queryKey, 
	std::optional<uint32_t> clientProtocol, 
	const NetAddress &clientAddr, 
	const ServerList::EntryContainer &servers, 
	const std::string &gamedir) :
	m_queryKey(queryKey),
	m_clientProtocol(clientProtocol),
	m_clientAddress(clientAddr),
	m_natBypassMode(natBypass),
	m_gamedir(gamedir),
	m_servers(servers)
{
}

void ClientQueryResponse::Serialize(BinaryOutputStream &stream, std::vector<NetAddress> &natServers) const
{
	stream.WriteString(ClientQueryResponse::Header);
	if (m_queryKey.has_value())
	{
		stream.WriteByte(0x7F);
		stream.Write<uint32_t>(m_queryKey.value());
		stream.WriteByte(0x00);
	}

	// TODO implement pagination mechanism to bypass MTU limit, when servers count are huge
	// but for November 2024, engine still does not supports such mechanism
	// for more information see CL_ServerList function in engine sources
	natServers.clear();
	for (const auto &[serverAddr, entry] : m_servers)
	{
		if (serverAddr.GetAddressFamily() != m_clientAddress.GetAddressFamily())
			continue;

		if (entry.NatBypassEnabled() != m_natBypassMode)
			continue;

		if (m_gamedir.compare(entry.GetGamedir()) != 0)
			continue;

		if (m_clientProtocol.has_value())
		{
			if (entry.GetProtocolVersion() != m_clientProtocol.value()) {
				continue;
			}
		}

		if (m_natBypassMode) {
			natServers.push_back(serverAddr);
		}
		stream.WriteNetAddress(serverAddr);
	}

	// write null address as an end of message marker
	stream.WriteByte(0x00, 6);
}
