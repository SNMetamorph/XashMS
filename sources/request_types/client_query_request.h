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

class ClientQueryRequest
{
public:
	~ClientQueryRequest() = default;
	static std::optional<ClientQueryRequest> Parse(BinaryInputStream& stream);

	bool ClientOutdated() const { return m_clientOutdated; };
	bool ClientBypassingNat() const { return m_clientBypassingNat; }
	const std::string &GetGamedir() const { return m_gamedir; }
	std::optional<uint32_t> GetQueryKey() const { return m_queryKey; };
	std::optional<uint32_t> GetProtocolVersion() const { return m_protocolVersion; }
	std::optional<std::string> GetClientVersion() const { return m_clientVersion; }

private:
	ClientQueryRequest() = default;

	bool ValidateInfostring(const InfostringData &data) const;

	bool m_clientOutdated;
	bool m_clientBypassingNat;
	std::string m_gamedir;
	std::optional<uint32_t> m_queryKey;
	std::optional<uint32_t> m_protocolVersion;
	std::optional<std::string> m_clientVersion;
};
