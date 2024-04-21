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

#include "infostring_data.h"
#include <fmt/core.h>

InfostringData::InfostringData(const std::string &data)
{
	Parse(data);
}

void InfostringData::Parse(const std::string &data)
{
	std::string key, value;
	key.reserve(32);
	value.reserve(32);

	const char *text = data.c_str();
	if (text[0] == '\\') {
		text += 1;
	}
	for (const char *p = text; *p != '\0'; p++) 
	{
		char character = *p;
		if (character != '\\') {
			key.push_back(character);
		}
		else 
		{
			for (p += 1; *p != '\0'; p++) 
			{
				character = *p;
				if (character != '\\') {
					value.push_back(character);
				}
				else {
					break;
				}
			}
			m_hashmap.insert_or_assign(key, value);
			key.clear();
			value.clear();
		}
	}
}

std::string InfostringData::ToString() const
{
	std::string result;
	for (const auto &entry : m_hashmap) {
		result += fmt::format("\\{}\\{}", entry.first, entry.second);
	}
	return result;
}

bool InfostringData::Contains(const char *key) const
{
	return m_hashmap.count(key) > 0;
}

const std::string &InfostringData::Get(const char *key)
{
	return m_hashmap[key];
}

void InfostringData::Insert(const char *key, const char *value)
{
	m_hashmap.insert_or_assign(key, value);
}
