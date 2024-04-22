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

#include "application.h"
#include "build.h"
#include "build_info.h"
#include <fmt/core.h>
#include <stdexcept>

#if BUILD_WIN32 == 1
#include <winsock2.h>
#endif

Application::Application() :
	m_argsParser("xash-ms", "1.0", argparse::default_arguments::help)
{
}

int Application::Run(int argc, char *argv[])
{
#if BUILD_WIN32 == 1
	WSADATA wsaData;
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
		return -1;
	}
#endif

	InitializeProgramArguments();
	if (argc == 1) 
	{
		fmt::print(m_argsParser.help().str());
		return 0;
	}
	
	try {
		PrintProgramTitle();
		m_argsParser.parse_args(argc, argv);
	}
	catch (const std::exception& err) {
		fmt::print("Arguments parsing error: {}\n", err.what());
		return -1;
	}

	if (m_argsParser.present("--ip")) {
		InitializeSocketIPv4();
	}
	if (m_argsParser.present("--ip6")) {
		InitializeSocketIPv6();
	}

	if (m_socket_ipv4 || m_socket_ipv6) 
	{
		fmt::print("Starting requests listening...\n\n");
		m_eventLoop = std::make_unique<EventLoop>(m_socket_ipv4, m_socket_ipv6);
		m_eventLoop->Run();
	}
	else 
	{
		fmt::print("Failed to initialize both IPv4 and IPv6 sockets, exiting.\n");
		return -1;
	}

#if BUILD_WIN32 == 1
	WSACleanup();
#endif
	return 0;
}

void Application::PrintProgramTitle()
{
	fmt::print("\nXashMS started: platform {}, architecture {}, commit {}/{}\n",
		BuildInfo::GetPlatform(),
		BuildInfo::GetArchitecture(),
		BuildInfo::GetCommitHash(), 
		BuildInfo::GetBranchName());
}

void Application::InitializeProgramArguments()
{
	m_argsParser.add_argument("-p", "--port")
		.help("number of port that will be used for incoming connections")
		.default_value(27010)
		.required()
		.scan<'d', int>();

	m_argsParser.add_argument("-ip", "--ip")
		.help("address of IPv4 interface, which will be listened for incoming packets");

	m_argsParser.add_argument("-ip6", "--ip6")
		.help("address of IPv6 interface, which will be listened for incoming packets");
}

void Application::InitializeSocketIPv4()
{
	NetAddress address(NetAddress::AddressFamily::IPv4);
	std::string addressString = m_argsParser.get<std::string>("--ip").c_str();
	uint16_t portNumber = m_argsParser.get<int>("--port");

	if (address.FromString(addressString.c_str(), portNumber))
	{
		try {
			m_socket_ipv4 = std::make_shared<Socket>(AF_INET, SOCK_DGRAM, 0);
			m_socket_ipv4->Bind(address);
			fmt::print("Server IPv4 address: {}:{}\n", address.ToString(), address.GetPort());
		}
		catch (const std::exception &ex) {
			m_socket_ipv4.reset();
			fmt::print("Failed to initialize IPv4 socket: {}\n", ex.what());
		}
	}
	else {
		fmt::print("Failed to parse IPv4 interface address\n");
	}
}

void Application::InitializeSocketIPv6()
{
	NetAddress address(NetAddress::AddressFamily::IPv6);
	std::string addressString = m_argsParser.get<std::string>("--ip6").c_str();
	uint16_t portNumber = m_argsParser.get<int>("--port");

	if (address.FromString(addressString.c_str(), portNumber)) 
	{
		try {
			m_socket_ipv6 = std::make_shared<Socket>(AF_INET6, SOCK_DGRAM, 0);
			m_socket_ipv6->Bind(address);
			fmt::print("Server IPv6 address: {}:{}\n", address.ToString(), address.GetPort());
		}
		catch (const std::exception &ex) {
			m_socket_ipv6.reset();
			fmt::print("Failed to initialize IPv6 socket: {}\n", ex.what());
		}
	}
	else {
		fmt::print("Failed to parse IPv6 interface address\n");
	}
}
