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

#include "server_append_request.h"

std::optional<ServerAppendRequest> ServerAppendRequest::Parse(BinaryInputStream &stream)
{
	InfostringData data;
	ServerAppendRequest object;
	std::string infostring;

	stream.SkipBytes(2); // skip header bytes
	if (stream.EndOfFile()) {
		return std::nullopt; // invalid request length
	}

	stream.ReadString(infostring); // this string in request is not null-terminated
	data.Parse(infostring);
	if (!object.ValidateInfostring(data)) {
		return std::nullopt; // request infostring correctness and fullness check failed
	}

	auto version = VersionInfo::Parse(data["version"].value());
	if (!version.has_value()) {
		return std::nullopt; // error while parsing version info
	}

	object.m_challenge = std::atoll(data["challenge"].value().c_str());
	object.m_serverVersion = version.value();
	object.m_infostringData = std::move(data);
	return object;
}

bool ServerAppendRequest::ValidateInfostring(const InfostringData &data)
{
	if (!data["challenge"].has_value()) {
		return false;
	}
	if (!data["protocol"].has_value()) {
		return false;
	}
	if (!data["players"].has_value()) {
		return false;
	}
	if (!data["max"].has_value()) {
		return false;
	}
	if (!data["bots"].has_value()) {
		return false;
	}
	if (!data["region"].has_value()) {
		return false;
	}
	if (!data["gamedir"].has_value()) {
		return false;
	}
	if (!data["map"].has_value()) {
		return false;
	}
	if (!data["version"].has_value()) {
		return false;
	}
	if (!data["os"].has_value()) {
		return false;
	}
	if (!data["product"].has_value()) {
		return false;
	}
	if (!data["type"].has_value()) {
		return false;
	}
	if (!data["password"].has_value()) {
		return false;
	}
	if (!data["secure"].has_value()) {
		return false;
	}
	if (!data["lan"].has_value()) {
		return false;
	}
	if (!data["nat"].has_value()) {
		return false;
	}
	return true;
}
