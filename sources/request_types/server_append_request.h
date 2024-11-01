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
#include "binary_input_stream.h"
#include "infostring_data.h"
#include <optional>
#include <stdint.h>

class ServerAppendRequest
{
public:
	~ServerAppendRequest() = default;
	static std::optional<ServerAppendRequest> Parse(BinaryInputStream &stream);

	uint32_t GetChallenge() const { return m_challenge; }
	const InfostringData &GetInfostringData() const { return m_infostringData; }

private:
	ServerAppendRequest() = default;

	bool ValidateInfostring(const InfostringData &data);

	uint32_t m_challenge;
	InfostringData m_infostringData;
};
