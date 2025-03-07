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
#include "version_info.h"
#include <vector>
#include <string>
#include <stdint.h>

class ConfigData
{
public:
	struct AdminEntry
	{
		std::string name;
		std::string password;
	};

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
	size_t GetAdminHashLength() const { return m_adminHashLength; }
	const std::string& GetAdminHashKey() const { return m_adminHashKey; }
	const std::string& GetAdminHashPersonal() const { return m_adminHashPersonal; }
	const std::vector<AdminEntry>& GetAdmins() const { return m_adminsList; }
	const VersionInfo& GetServerMinimalVersion() const { return m_serverMinimalVersion; }
	const VersionInfo& GetClientMinimalVersion() const { return m_clientMinimalVersion; }

private:
	size_t m_serverCountQuota;
	size_t m_adminHashLength;
	float m_cleanupInterval;
	float m_serverTimeoutInterval;
	float m_challengeTimeoutInterval;
	std::string m_adminHashKey;
	std::string m_adminHashPersonal;
	std::vector<AdminEntry> m_adminsList;
	VersionInfo m_serverMinimalVersion;
	VersionInfo m_clientMinimalVersion;
};
