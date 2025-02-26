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
#include <string>
#include <string_view>
#include <optional>
#include <stdint.h>

class VersionInfo
{
public:
	VersionInfo();
	VersionInfo(uint32_t major, uint32_t minor, std::optional<uint32_t> patch);

	bool operator<(const VersionInfo &rhs) const;
	bool operator==(const VersionInfo &rhs) const;
	bool operator!=(const VersionInfo &rhs) const;
	bool operator>(const VersionInfo &rhs) const;
	bool operator<=(const VersionInfo &rhs) const;
	bool operator>=(const VersionInfo &rhs) const;

	static std::optional<VersionInfo> Parse(std::string_view text);
	std::string ToString() const;

	uint32_t major;
	uint32_t minor;
	std::optional<uint32_t> patch;
};
