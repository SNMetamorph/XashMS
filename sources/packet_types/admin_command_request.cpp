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

#include "admin_command_request.h"

std::optional<AdminCommandRequest> AdminCommandRequest::Parse(BinaryInputStream &stream, size_t hashLength)
{
	AdminCommandRequest object;
	object.m_challenge = stream.Read<uint32_t>();
	stream.ReadBytes(object.m_hash.data(), hashLength);
	stream.ReadString(object.m_command);

	if (stream.Underflowed()) {
		return std::nullopt;
	}
	return object;
}
