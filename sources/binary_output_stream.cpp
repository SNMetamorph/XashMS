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
	m_buffer(nullptr),
	m_offset(0),
	m_bufferSize(0),
	m_dynamicBuffer(buffer)
{
	buffer.clear();
}

BinaryOutputStream::BinaryOutputStream(uint8_t *buffer, size_t bufferSize) :
	m_buffer(buffer),
	m_offset(0),
	m_bufferSize(bufferSize),
	m_dynamicBuffer(std::nullopt)
{
}

const uint8_t *BinaryOutputStream::GetBuffer() const
{
	return m_dynamicBuffer.has_value() ? m_dynamicBuffer.value().get().data() : m_buffer;
}

size_t BinaryOutputStream::GetLength() const
{
	return m_dynamicBuffer.has_value() ? m_dynamicBuffer.value().get().size() : m_offset;
}

bool BinaryOutputStream::WriteString(const char *text, bool includeNull)
{
	size_t length = std::strlen(text);
	return WriteBytes(text, includeNull ? length + 1 : length);
}

bool BinaryOutputStream::WriteBytes(const void *data, size_t count)
{
	const uint8_t *sourceBuffer = reinterpret_cast<const uint8_t*>(data);
	if (m_dynamicBuffer.has_value()) 
	{
		auto &dynamicBuffer = m_dynamicBuffer.value().get();
		dynamicBuffer.reserve(dynamicBuffer.size() + count);
		dynamicBuffer.insert(dynamicBuffer.end(), sourceBuffer, sourceBuffer + count);
		return true;
	}
	else
	{
		if (m_offset + count <= m_bufferSize)
		{
			std::memcpy(m_buffer + m_offset, sourceBuffer, count);
			m_offset += count;
			return true;
		}
	}
	return false;
}

bool BinaryOutputStream::WriteByte(uint8_t value, size_t repeats)
{
	if (m_dynamicBuffer.has_value()) 
	{
		auto &dynamicBuffer = m_dynamicBuffer.value().get();
		dynamicBuffer.reserve(dynamicBuffer.size() + repeats);
		dynamicBuffer.insert(dynamicBuffer.end(), repeats, value);
		return true;
	}
	else
	{
		if (m_offset + repeats <= m_bufferSize)
		{
			std::memset(m_buffer + m_offset, value, repeats);
			m_offset += repeats;
			return true;
		}
	}
	return false;
}

bool BinaryOutputStream::WriteNetAddress(const NetAddress &address)
{
	auto span = address.GetAddressSpan();
	WriteBytes(span.first, span.second);
	WriteByte((address.GetPort() >> 8) & 0xFF); // write port number as big endian 16-bit integer
	return WriteByte(address.GetPort() & 0xFF);
}
