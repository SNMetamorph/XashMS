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

#include "binary_input_stream.h"
#include <algorithm>
#include <cstring>

BinaryInputStream::BinaryInputStream(const void *address, size_t dataSize) :
	m_bufferAddress(reinterpret_cast<const uint8_t*>(address)),
	m_bufferSize(dataSize),
	m_currentOffset(0),
	m_underflowFlag(false)
{
}

bool BinaryInputStream::SkipBytes(size_t count)
{
	size_t remainingBytes = m_bufferSize - m_currentOffset;
	if (remainingBytes >= count) {
		m_currentOffset += count;
		return true;
	}
	m_underflowFlag = true;
	return false;
}

bool BinaryInputStream::SkipString()
{
	size_t remainingBytes = m_bufferSize - m_currentOffset;
	if (remainingBytes > 0)
	{
		for (size_t i = 0; i < remainingBytes; i++)
		{
			char character;
			ReadData(&character, sizeof(character));
			if (character == '\0') {
				break;
			}
		}
		return true;
	}
	else {
		m_underflowFlag = true;
	}
	return false;
}

bool BinaryInputStream::ReadString(std::string &dest)
{
	dest.clear();
	dest.reserve(16);
	size_t remainingBytes = m_bufferSize - m_currentOffset;
	if (remainingBytes > 0)
	{
		for (size_t i = 0; i < remainingBytes; i++)
		{
			char character;
			ReadData(&character, sizeof(character));
			if (character != '\0') {
				dest.push_back(character);
			}
			else {
				break;
			}
		}
		return true;
	}
	else {
		m_underflowFlag = true;
	}
	return false;
}

bool BinaryInputStream::ReadBytes(void *destBuffer, size_t count)
{
	size_t remainingBytes = m_bufferSize - m_currentOffset;
	if (remainingBytes >= count) 
	{
		ReadData(destBuffer, count);
		return true;
	}
	else {
		m_underflowFlag = true;
	}
	return false;
}

bool BinaryInputStream::EndOfFile() const
{
	return (m_bufferSize - m_currentOffset) == 0;
}

bool BinaryInputStream::Underflowed() const
{
	return m_underflowFlag;
}

size_t BinaryInputStream::GetBufferSize() const
{
	return m_bufferSize;
}

size_t BinaryInputStream::GetPosition() const
{
	return m_currentOffset;
}

size_t BinaryInputStream::ReadData(void *destBuffer, size_t count)
{
	size_t remainingBytes = std::min(count, m_bufferSize - m_currentOffset);
	std::memcpy(destBuffer, m_bufferAddress + m_currentOffset, remainingBytes);
	m_currentOffset += remainingBytes;
	return remainingBytes;
}
