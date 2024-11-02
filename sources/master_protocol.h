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

namespace MasterProtocol
{
	constexpr const char *removeServer = "b\n";
	constexpr const char *queryPacketHeader = "\xff\xff\xff\xff\x66\n";
	constexpr const char *challengePacketHeader = "\xff\xff\xff\xff\x73\x0a";
	constexpr const char *natBypassPacketHeader = "\xff\xff\xff\xff\x63\x20";
	constexpr const char *fakeServerInfoHeader = "\xff\xff\xff\xffinfo\n";
}
