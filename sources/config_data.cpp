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

#include "config_data.h"
#include <rapidjson/rapidjson.h>
#include <rapidjson/document.h>

ConfigData::ConfigData() :
	m_serverCountQuota(14),
	m_adminHashLength(64),
	m_packetRateLimit(100),
	m_cleanupInterval(10.0f),
	m_rateLimitBanTime(15.0f),
	m_serverTimeoutInterval(360.0f),
	m_challengeTimeoutInterval(15.0f),
	m_adminHashKey("Half-Life"),
	m_adminHashPersonal("Freeman")
{
}

bool ConfigData::Parse(const std::string &jsonData)
{
	rapidjson::Document document;
	document.Parse(jsonData.c_str());

	if (document.HasParseError() || !document.IsObject()) {
		return false;
	}

	// maybe use JSON schema validation instead?
	if (!document.HasMember("max_servers_per_ip") ||
		!document.HasMember("cleanup_interval") ||
		!document.HasMember("server_timeout_interval") ||
		!document.HasMember("challenge_timeout_interval") ||
		!document.HasMember("admin_hash_length") ||
		!document.HasMember("admin_hash_key") ||
		!document.HasMember("admin_hash_personal")) 
	{
		return false;
	}

	if (!document["max_servers_per_ip"].IsInt() ||
		!document["cleanup_interval"].IsFloat() ||
		!document["server_timeout_interval"].IsFloat() ||
		!document["challenge_timeout_interval"].IsFloat() ||
		!document["admin_hash_length"].IsInt() ||
		!document["admin_hash_key"].IsString() ||
		!document["admin_hash_personal"].IsString())
	{
		return false;
	}

	if (!document.HasMember("admins") || !document["admins"].IsArray()) {
		return false;
	}

	const rapidjson::Value &adminsArray = document["admins"];
	for (size_t i = 0; i < adminsArray.Size(); i++)
	{
		const rapidjson::Value &adminEntry = adminsArray[i];
		if (adminEntry.HasMember("name") && adminEntry.HasMember("password"))
		{
			if (adminEntry["name"].IsString() && adminEntry["password"].IsString()) {
				m_adminsList.push_back({ adminEntry["name"].GetString(), adminEntry["password"].GetString() });
			}
		}
	}

	m_adminHashLength = document["admin_hash_length"].GetInt();
	m_serverCountQuota = document["max_servers_per_ip"].GetInt();
	m_cleanupInterval = document["cleanup_interval"].GetFloat();
	m_serverTimeoutInterval = document["server_timeout_interval"].GetFloat();
	m_challengeTimeoutInterval = document["challenge_timeout_interval"].GetFloat();
	m_adminHashKey = document["admin_hash_key"].GetString();
	m_adminHashPersonal = document["admin_hash_personal"].GetString();
	return true;
}
