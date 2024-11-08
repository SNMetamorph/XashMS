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
#include <array>
#include <string>
#include <optional>
#include <stdint.h>

class AdminCommandRequest
{
public:
	static constexpr const char *Header = "admin";

	~AdminCommandRequest() = default;

	static std::optional<AdminCommandRequest> Parse(BinaryInputStream &stream, size_t hashLength);
	uint32_t GetMasterChallenge() const { return m_challenge; }
	const std::string& GetCommand() const { return m_command; }
	const uint8_t *GetHash() const { return m_hash.data(); }

private:
	AdminCommandRequest() = default;

	uint32_t m_challenge;
	std::string m_command;
	std::array<uint8_t, 64> m_hash;
};
