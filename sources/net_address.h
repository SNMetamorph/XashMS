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
#include <stdint.h>
#include <string>
#include <string_view>
#include <array>
#include <utility>

#if BUILD_WIN32 == 1
#include <winsock2.h>
#include <ws2tcpip.h>
#elif BUILD_POSIX == 1
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#endif

class NetAddress
{
public:
	friend class NetAddressHash;
	friend class NetAddressPortHash;
	enum class AddressFamily
	{
		IPv4,
		IPv6
	};

	NetAddress(AddressFamily family);
	~NetAddress() = default;
	NetAddress(const NetAddress&) = default;
	NetAddress(NetAddress&&) noexcept = default;
	NetAddress &operator=(const NetAddress&) = default;
	NetAddress &operator=(NetAddress&&) noexcept = default;
	bool operator==(const NetAddress &rhs) const;

	uint16_t GetPort() const { return m_port; }
	AddressFamily GetAddressFamily() const { return m_family; }
	std::pair<const uint8_t*, size_t> GetAddressSpan() const;
	std::string ToString() const;

	bool FromString(const char *address, uint16_t port);
	bool Equals(const NetAddress &lhs, bool includePort = false) const;
	void ToSockadr(sockaddr_in *address) const;
	void ToSockadr(sockaddr_in6 *address) const;
	void FromSockadr(const sockaddr_in *address);
	void FromSockadr(const sockaddr_in6 *address);

private:
	uint16_t m_port;
	AddressFamily m_family;
	std::array<uint8_t, 16> m_addressData;
};

class NetAddressHash
{
public:
	std::size_t operator()(const NetAddress &address) const noexcept
    {
		std::size_t addrSize = address.GetAddressSpan().second;
		return std::hash<std::string_view>{}({ reinterpret_cast<const char*>(address.m_addressData.data()), addrSize });
    }
};

class NetAddressPortHash
{
public:
	std::size_t operator()(const NetAddress &address) const noexcept
    {
		std::size_t addrSize = address.GetAddressSpan().second;
        std::size_t addrHash = std::hash<std::string_view>{}({ reinterpret_cast<const char*>(address.m_addressData.data()), addrSize });
        std::size_t portHash = std::hash<uint16_t>{}(address.m_port);
        return addrHash ^ (portHash << 1);
    }
};
