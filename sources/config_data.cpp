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
	m_cleanupInterval(10.0f),
	m_serverTimeoutInterval(360.0f),
	m_challengeTimeoutInterval(15.0f)
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
		!document.HasMember("challenge_timeout_interval")) 
	{
		return false;
	}

	if (!document["max_servers_per_ip"].IsInt() ||
		!document["cleanup_interval"].IsFloat() ||
		!document["server_timeout_interval"].IsFloat() ||
		!document["challenge_timeout_interval"].IsFloat())
	{
		return false;
	}

	m_serverCountQuota = document["max_servers_per_ip"].GetInt();
	m_cleanupInterval = document["cleanup_interval"].GetFloat();
	m_serverTimeoutInterval = document["server_timeout_interval"].GetFloat();
	m_challengeTimeoutInterval = document["challenge_timeout_interval"].GetFloat();
	return true;
}