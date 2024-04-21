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
#include <stdint.h>

class InfostringData
{
public:
	InfostringData() = default;
	InfostringData(const std::string &data);

	void Parse(const std::string &data);
	bool Contains(const char *key) const;
	void Insert(const char *key, const char *value);
	const std::string &Get(const char *key);
	std::string ToString() const;

private:
	std::unordered_map<std::string, std::string> m_hashmap;
};
