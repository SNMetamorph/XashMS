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

#include "binary_output_stream.h"
#include <cstring>

BinaryOutputStream::BinaryOutputStream(std::vector<uint8_t> &buffer) :
	m_buffer(buffer)
{
	buffer.clear();
}

void BinaryOutputStream::WriteString(const char *text, bool includeNull)
{
	size_t length = std::strlen(text);
	m_buffer.insert(m_buffer.end(), text, text + length);
	if (includeNull) {
		m_buffer.push_back(0x0);
	}
}

void BinaryOutputStream::WriteBytes(const void *data, size_t count)
{
	const uint8_t *sourceBuffer = reinterpret_cast<const uint8_t*>(data);
	m_buffer.insert(m_buffer.end(), sourceBuffer, sourceBuffer + count);
}

void BinaryOutputStream::WriteByte(uint8_t value, size_t repeats)
{
	m_buffer.insert(m_buffer.end(), repeats, value);
}

void BinaryOutputStream::WriteNetAddress(const NetAddress &address)
{
	auto span = address.GetAddressSpan();
	WriteBytes(span.first, span.second);

	// write port number as big endian 16-bit integer
	WriteByte((address.GetPort() >> 8) & 0xFF);
	WriteByte(address.GetPort() & 0xFF);
}
