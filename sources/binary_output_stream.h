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
#include "net_address.h"
#include <vector>
#include <string>
#include <stdint.h>

class BinaryOutputStream
{
public:
	BinaryOutputStream(std::vector<uint8_t> &buffer);

	void WriteString(const char *text, bool includeNull = false);
	void WriteBytes(const void *data, size_t count);
	void WriteByte(uint8_t value, size_t repeats = 1);
	void WriteNetAddress(const NetAddress &address);

	template<class T> void Write(const T& value) 
	{
		static_assert(std::is_scalar<T>(), "type should be scalar");
		const uint8_t *dataAddress = reinterpret_cast<const uint8_t*>(&value);
		m_buffer.reserve(m_buffer.size() + sizeof(T));
		for (size_t i = 0; i < sizeof(T); i++) {
			m_buffer.push_back(dataAddress[i]);
		}
	}

private:
	std::vector<uint8_t> &m_buffer;
};
