/*
Copyright (C) 2025 SNMetamorph

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
#include <tuple>
#include <optional>
#include <string>
#include <string_view>
#include <stdint.h>

class VersionInfo
{
public:
	VersionInfo();
	VersionInfo(uint32_t major, uint32_t minor, std::optional<uint32_t> patch);

	static std::optional<VersionInfo> Parse(std::string_view text);
	std::string ToString() const;

	uint32_t major;
	uint32_t minor;
	std::optional<uint32_t> patch;
};

inline bool operator<(const VersionInfo& a, const VersionInfo& b) 
{
    return std::tie(a.major, a.minor, a.patch) < std::tie(b.major, b.minor, b.patch);
}
