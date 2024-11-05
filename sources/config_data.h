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
#include <vector>
#include <string>
#include <stdint.h>

class ConfigData
{
public:
	ConfigData();
	ConfigData(const ConfigData&) = default;
	ConfigData(ConfigData&&) noexcept = default;
	ConfigData& operator=(const ConfigData&) = default;
	ConfigData& operator=(ConfigData&&) noexcept = default;

	bool Parse(const std::string &jsonData);
	size_t GetServerCountQuota() const { return m_serverCountQuota; }
	float GetCleanupInterval() const { return m_cleanupInterval; }
	float GetServerTimeoutInterval() const { return m_serverTimeoutInterval; }
	float GetChallengeTimeoutInterval() const { return m_challengeTimeoutInterval; }

private:
	size_t m_serverCountQuota;
	float m_cleanupInterval;
	float m_serverTimeoutInterval;
	float m_challengeTimeoutInterval;
};
