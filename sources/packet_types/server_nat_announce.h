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
#include "net_address.h"
#include "binary_output_stream.h"

class ServerNatAnnounce
{
public:
	static constexpr const char *Header = "\xff\xff\xff\xff" "c ";

	ServerNatAnnounce(const NetAddress &clientAddress);
	void Serialize(BinaryOutputStream &stream) const;

private:
	NetAddress m_clientAddress;
};
