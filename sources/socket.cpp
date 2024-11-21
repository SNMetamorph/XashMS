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

#include "socket.h"
#include <stdexcept>
#include <cstring>

#if BUILD_WIN32 == 1
#include <ws2tcpip.h>
#elif BUILD_POSIX == 1
#include <unistd.h>
#endif

Socket::Socket(int32_t af, int32_t type, int32_t protocol)
{
	m_addressFamily = af;
	m_socket = socket(af, type, protocol);
	if (m_socket == EVUTIL_INVALID_SOCKET) {
		throw std::runtime_error("socket() returned invalid handle");
	}
	if (af == AF_INET6) 
	{
		int flag = 1;
		if (setsockopt(m_socket, IPPROTO_IPV6, IPV6_V6ONLY, (char *)&flag, sizeof(flag)) != 0) {
			throw std::runtime_error("IPV6_V6ONLY setsockopt failed");
		}
	}
	m_dataBuffer.resize(4096);
}

Socket::~Socket()
{
	evutil_closesocket(m_socket);
}

Socket::Socket(Socket &&rhs) noexcept
{
	m_socket = rhs.m_socket;
	m_addressFamily = rhs.m_addressFamily;
	rhs.m_socket = NULL;
	rhs.m_addressFamily = NULL;
}

Socket &Socket::operator=(Socket &&rhs) noexcept
{
	m_socket = rhs.m_socket;
	m_addressFamily = rhs.m_addressFamily;
	rhs.m_socket = NULL;
	rhs.m_addressFamily = NULL;
	return *this;
}

void Socket::Bind(const NetAddress &addr)
{
	sockaddr *actualAddr;
	size_t sockaddrSize;
	sockaddr_in address_v4;
	sockaddr_in6 address_v6;

	std::memset(&address_v4, 0, sizeof(address_v4));
	std::memset(&address_v6, 0, sizeof(address_v6));
	if (addr.GetAddressFamily() == NetAddress::AddressFamily::IPv4) 
	{
		actualAddr = reinterpret_cast<sockaddr*>(&address_v4);
		sockaddrSize = sizeof(address_v4);
		addr.ToSockadr(&address_v4);
	}
	else
	{
		actualAddr = reinterpret_cast<sockaddr*>(&address_v6);
		sockaddrSize = sizeof(address_v6);
		addr.ToSockadr(&address_v6);
	}
	
	if (bind(m_socket, actualAddr, sockaddrSize) != 0) {
		throw std::runtime_error("failed to bind socket");
	}
}

NetAddress Socket::RecvFrom()
{
	sockaddr *actualAddr;
	socklen_t sockaddrSize;
	sockaddr_in address_v4;
	sockaddr_in6 address_v6;

	std::memset(&address_v4, 0, sizeof(address_v4));
	std::memset(&address_v6, 0, sizeof(address_v6));
	if (m_addressFamily == AF_INET) {
		actualAddr = reinterpret_cast<sockaddr*>(&address_v4);
		sockaddrSize = sizeof(address_v4);
	}
	else {
		actualAddr = reinterpret_cast<sockaddr*>(&address_v6);
		sockaddrSize = sizeof(address_v6);
	}

	m_dataBuffer.resize(m_dataBuffer.capacity());
	char *dataAddr = reinterpret_cast<char*>(m_dataBuffer.data());
	int32_t bytesCount = recvfrom(m_socket, dataAddr, m_dataBuffer.capacity(), 0, actualAddr, &sockaddrSize);
	if (bytesCount == -1) {
		throw std::runtime_error("recvfrom() returned error status");
	}
	
	m_dataBuffer.resize(bytesCount);
	if (m_addressFamily == AF_INET) 
	{
		NetAddress addr(NetAddress::AddressFamily::IPv4);
		addr.FromSockadr(&address_v4);
		return addr;
	}
	else 
	{
		NetAddress addr(NetAddress::AddressFamily::IPv6);
		addr.FromSockadr(&address_v6);
		return addr;
	}
}

bool Socket::SendTo(const NetAddress &destination, const std::vector<uint8_t> &data)
{
	return SendTo(destination, data.data(), data.size());
}

bool Socket::SendTo(const NetAddress &destination, const uint8_t *buffer, size_t dataSize)
{
	sockaddr *actualAddr;
	size_t sockaddrSize;
	sockaddr_in address_v4;
	sockaddr_in6 address_v6;

	std::memset(&address_v4, 0, sizeof(address_v4));
	std::memset(&address_v6, 0, sizeof(address_v6));
	if (destination.GetAddressFamily() == NetAddress::AddressFamily::IPv4) 
	{
		actualAddr = reinterpret_cast<sockaddr*>(&address_v4);
		sockaddrSize = sizeof(address_v4);
		destination.ToSockadr(&address_v4);
	}
	else
	{
		actualAddr = reinterpret_cast<sockaddr*>(&address_v6);
		sockaddrSize = sizeof(address_v6);
		destination.ToSockadr(&address_v6);
	}

	const char *dataAddress = reinterpret_cast<const char*>(buffer);
	if (sendto(m_socket, dataAddress, dataSize, 0, actualAddr, sockaddrSize) != dataSize) {
		return false;
	}
	return true;
}
