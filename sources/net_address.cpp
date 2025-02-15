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

#include "net_address.h"
#include <fmt/core.h>
#include <event2/util.h>
#include <stdint.h>
#include <stdexcept>

NetAddress::NetAddress(AddressFamily family) :
	m_port(0),
	m_family(family)
{
	m_addressData.fill(0x0);
}

bool NetAddress::FromString(const char *address, uint16_t port)
{
	in_addr addr_v4;
	in6_addr addr_v6;

	if (m_family == AddressFamily::IPv4) 
	{
		if (!evutil_inet_pton(AF_INET, address, &addr_v4)) {
			return false;
		}
		std::memcpy(m_addressData.data(), &addr_v4, 4);
	}
	else 
	{
		if (!evutil_inet_pton(AF_INET6, address, &addr_v6)) {
			return false;
		}
		std::memcpy(m_addressData.data(), &addr_v6, 16);
	}

	m_port = port;
	return true;
}

bool NetAddress::operator==(const NetAddress &rhs) const
{
	return Equals(rhs, true);
}

std::pair<const uint8_t*, size_t> NetAddress::GetAddressSpan() const
{
	return std::pair<const uint8_t*, size_t>(m_addressData.data(), m_family == AddressFamily::IPv6 ? 16 : 4);
}

std::string NetAddress::ToString() const
{
	char buffer[INET6_ADDRSTRLEN];
	const int32_t addrFamily = (m_family == AddressFamily::IPv4) ? AF_INET : AF_INET6;
	if (evutil_inet_ntop(addrFamily, m_addressData.data(), buffer, sizeof(buffer))) {
		return std::string(buffer);
	}
	return std::string("unknown");
}

bool NetAddress::Equals(const NetAddress &lhs, bool includePort) const
{
	if (m_family != lhs.m_family)
		return false;
	if (includePort && m_port != lhs.m_port)
		return false;

	size_t addressLength = (m_family == AddressFamily::IPv6) ? 16 : 4;
	if (std::memcmp(m_addressData.data(), lhs.m_addressData.data(), addressLength) == 0) {
		return true;
	}
	return false;
}

void NetAddress::ToSockadr(sockaddr_in *address) const
{
	if (m_family == AddressFamily::IPv4) 
	{
		address->sin_family = AF_INET;
		address->sin_port = htons(m_port);
		std::memcpy(&address->sin_addr.s_addr, m_addressData.data(), 4);
	}
	else {
		throw std::runtime_error("sockaddr address family mismatching");
	}
}

void NetAddress::ToSockadr(sockaddr_in6 *address) const
{
	if (m_family == AddressFamily::IPv6) 
	{
		address->sin6_family = AF_INET6;
		address->sin6_port = htons(m_port);
		std::memcpy(&address->sin6_addr, m_addressData.data(), 16);
	}
	else {
		throw std::runtime_error("sockaddr address family mismatching");
	}
}

void NetAddress::FromSockadr(const sockaddr_in *address)
{
	if (m_family == AddressFamily::IPv4) 
	{
		const sockaddr_in *address_v4 = reinterpret_cast<const sockaddr_in*>(address);
		std::memcpy(m_addressData.data(), &address_v4->sin_addr.s_addr, 4);
		m_port = ntohs(address_v4->sin_port);
	}
	else {
		throw std::runtime_error("sockaddr address family mismatching");
	}
}

void NetAddress::FromSockadr(const sockaddr_in6 *address)
{
	if (m_family == AddressFamily::IPv6) 
	{
		const sockaddr_in6 *address_v6 = reinterpret_cast<const sockaddr_in6*>(address);
		std::memcpy(m_addressData.data(), &address_v6->sin6_addr, 16);
		m_port = ntohs(address_v6->sin6_port);
	}
	else {
		throw std::runtime_error("sockaddr address family mismatching");
	}
}

std::optional<NetAddress> NetAddress::Parse(std::string_view address, uint16_t port)
{
	in_addr addr_v4;
	in6_addr addr_v6;
	if (evutil_inet_pton(AF_INET, address.data(), &addr_v4))
	{
		NetAddress result(AddressFamily::IPv4);
		std::memcpy(result.m_addressData.data(), &addr_v4, 4);
		result.m_port = port;
		return result;
	}
	else if (evutil_inet_pton(AF_INET6, address.data(), &addr_v6)) 
	{
		NetAddress result(AddressFamily::IPv6);
		std::memcpy(result.m_addressData.data(), &addr_v6, 16);
		result.m_port = port;
		return result;
	}
	return std::nullopt;
}
