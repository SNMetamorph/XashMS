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
#include "build.h"
#include "net_address.h"
#include <event2/util.h>
#include <vector>
#include <stdint.h>

#if BUILD_WIN32 == 1
#include <winsock2.h>
#elif BUILD_POSIX == 1
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#endif

class Socket
{
public:
	Socket(int32_t af, int32_t type, int32_t protocol);
	~Socket();
	Socket(Socket&& rhs) noexcept;
	Socket& operator=(Socket&& rhs) noexcept;

	void Bind(const NetAddress &addr);
	NetAddress RecvFrom();
	bool SendTo(const NetAddress &destination, const uint8_t *buffer, size_t dataSize);
	const std::vector<uint8_t> &GetDataBuffer() const { return m_dataBuffer; };
	evutil_socket_t GetDescriptor() const { return m_socket; }

private:
	Socket(const Socket&) = delete;
	Socket& operator=(const Socket&) = delete;

	evutil_socket_t m_socket;
	int32_t m_addressFamily;
	std::vector<uint8_t> m_dataBuffer;
};
