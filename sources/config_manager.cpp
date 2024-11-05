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

#include "config_manager.h"
#include <iostream>
#include <fstream>

bool ConfigManager::ParseConfig(const std::filesystem::path &configPath)
{
	std::ifstream fileStream(configPath);
	if (fileStream.is_open()) 
	{
		std::string jsonData;
		jsonData.assign(std::istreambuf_iterator<char>(fileStream), std::istreambuf_iterator<char>());
		return m_configData.Parse(jsonData);
	}
	return false;
}
