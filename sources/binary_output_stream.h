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
#include <optional>
#include <functional>
#include <stdint.h>

class BinaryOutputStream
{
public:
	BinaryOutputStream(std::vector<uint8_t> &buffer);
	BinaryOutputStream(uint8_t *buffer, size_t bufferSize);

	const uint8_t *GetBuffer() const;
	size_t GetLength() const;
	bool WriteString(const char *text, bool includeNull = false);
	bool WriteBytes(const void *data, size_t count);
	bool WriteByte(uint8_t value, size_t repeats = 1);
	bool WriteNetAddress(const NetAddress &address);

	template<class T> bool Write(const T& value) 
	{
		static_assert(std::is_scalar<T>(), "type should be scalar");
		return WriteBytes(reinterpret_cast<const uint8_t*>(&value), sizeof(T));
	}

private:
	uint8_t *m_buffer;
	size_t m_offset;
	size_t m_bufferSize;
	std::optional<std::reference_wrapper<std::vector<uint8_t>>> m_dynamicBuffer; // maybe just use pointer? no.
};
