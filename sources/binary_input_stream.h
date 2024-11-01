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
#include <stdint.h>
#include <type_traits>

class BinaryInputStream
{
public:
	BinaryInputStream(const void *address, size_t dataSize);

	template<class T> T Read() 
	{
		static_assert(std::is_scalar<T>(), "type should be scalar");
		T temp;
		ReadBytes(&temp, sizeof(T));
		return temp;
	}

	size_t SkipBytes(size_t count);
	bool SkipString();
	bool ReadString(std::string &dest);
	bool EndOfFile() const;
	size_t GetBufferSize() const;
	size_t GetPosition() const;

private:
	size_t ReadBytes(void *destBuffer, size_t count);

	size_t m_bufferSize;
	size_t m_currentOffset;
	const uint8_t *m_bufferAddress;
};
