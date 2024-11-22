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

#include "server_nat_announce.h"
#include <fmt/core.h>

ServerNatAnnounce::ServerNatAnnounce(const NetAddress &clientAddress) :
	m_clientAddress(clientAddress)
{
}

void ServerNatAnnounce::Serialize(BinaryOutputStream &stream) const
{
	std::string addrString = fmt::format("{}:{}", m_clientAddress.ToString(), m_clientAddress.GetPort());
	stream.WriteString(ServerNatAnnounce::Header);
	stream.WriteBytes(addrString.c_str(), addrString.size());
}
