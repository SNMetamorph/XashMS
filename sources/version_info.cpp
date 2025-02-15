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

#include "version_info.h"
#include <string_view>
#include <scn/scan.h>
#include <fmt/core.h>

VersionInfo::VersionInfo() :
	major(0), minor(0), patch(std::nullopt)
{
}

VersionInfo::VersionInfo(uint32_t _major, uint32_t _minor, std::optional<uint32_t> _patch) :
	major(_major), minor(_minor), patch(_patch)
{
}

std::optional<VersionInfo> VersionInfo::Parse(std::string_view text)
{
	auto s1 = scn::scan<uint32_t, uint32_t>(text, "{}.{}");
	if (s1)
	{
		auto [major, minor] = s1->values();
		auto s2 = scn::scan<uint32_t>(s1->range(), ".{}");
		
		VersionInfo result;
		if (s2.has_value()) {
			result.patch = s2->value();
		}
		else {
			result.patch = std::nullopt;
		}

		result.major = major;
		result.minor = minor;
		return result;
	}
	return std::nullopt;
}

std::string VersionInfo::ToString() const
{
	if (patch.has_value()) {
		return fmt::format("{}.{}.{}", major, minor, patch.value());
	}
	return fmt::format("{}.{}", major, minor);
}
