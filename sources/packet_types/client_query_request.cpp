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
#include "client_query_request.h"

// such function is an experimental approach to RAII without exceptions
std::optional<ClientQueryRequest> ClientQueryRequest::Parse(BinaryInputStream &stream)
{
	ClientQueryRequest object;
	std::string queryInfo;

	stream.SkipBytes(2); // skip header
	if (stream.EndOfFile()) {
		return std::nullopt;
	}

	if (!stream.SkipString()) { // placeholder addr, ignored
		return std::nullopt;
	}

	if (!stream.ReadString(queryInfo)) {
		return std::nullopt; 
	}

	size_t slashPos = queryInfo.find_first_of('\\');
	if (slashPos == std::string::npos) {
		return std::nullopt;
	}

	InfostringData data(queryInfo);
	if (!object.ValidateInfostring(data)) {
		return std::nullopt; 
	}

	if (data["key"].has_value())
	{
		auto scan = scn::scan_int<uint32_t>(data["key"].value(), 16);
		if (scan.has_value()) {
			object.m_queryKey = scan->value();
		}
		else {
			return std::nullopt; // invalid data in query key
		}
	}
	else {
		object.m_queryKey = std::nullopt;
	}
	
	if (data["protocol"].has_value()) 
	{
		auto scan = scn::scan_int<uint32_t>(data["protocol"].value());
		if (scan.has_value()) {
			object.m_protocolVersion = scan->value();
		}
		else {
			return std::nullopt; // invalid protocol number
		}
	}
	else {
		object.m_protocolVersion = std::nullopt;
	}

	object.m_clientOutdated = !data["clver"].has_value();
	object.m_clientBypassingNat = data["nat"].value().compare("0") != 0;
	object.m_clientVersion = data["clver"];
	object.m_gamedir = data["gamedir"].value();
	return object;
}

bool ClientQueryRequest::ValidateInfostring(const InfostringData &data) const
{
	if (!data["gamedir"].has_value()) {
		return false;
	}
	if (!data["nat"].has_value()) {
		return false;
	}
	return true;
}
