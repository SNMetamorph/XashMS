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
#include <string>
#include <unordered_map>
#include <optional>
#include <stdint.h>

class InfostringData
{
public:
	InfostringData() = default;
	InfostringData(const std::string &data);

	void Parse(const std::string &data);
	void Insert(const char *key, const char *value);
	void Insert(const std::string &key, const std::string &value);
	std::string ToString() const;
	std::optional<std::string> operator[](const char *key) const;
	std::optional<std::string> operator[](const std::string &key) const;

private:
	std::unordered_map<std::string, std::string> m_hashmap;
};
