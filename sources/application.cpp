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
#include "utils.h"
#include <stdexcept>
#include <cstdio>

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
		Utils::Log(m_argsParser.help().str());
		return 0;
	}
	
	try {
		PrintProgramTitle();
		m_argsParser.parse_args(argc, argv);
		if (m_argsParser["--unbuffered"] == true)
		{
			std::setbuf(stdout, nullptr);
			std::setbuf(stderr, nullptr);
		}
	}
	catch (const std::exception& err) {
		Utils::Log("Arguments parsing error: {}\n", err.what());
		return -1;
	}

	if (m_argsParser.present("--ip")) {
		InitializeSocketInet();
	}
	if (m_argsParser.present("--ip6")) {
		InitializeSocketInet6();
	}

	m_configManager = std::make_shared<ConfigManager>();
	if (m_configManager->ParseConfig(m_argsParser.get<std::string>("--config-file"))) {
		Utils::Log("Configuration file loaded\n");
	}

	if (m_socketInet || m_socketInet6) 
	{
		Utils::Log("Starting listening for requests...\n");
		m_eventLoop = std::make_unique<EventLoop>(m_socketInet, m_socketInet6, m_configManager);
		m_eventLoop->Run();
		Utils::Log("Shutting down...\n");
	}
	else 
	{
		Utils::Log("Failed to initialize both IPv4 and IPv6 sockets, exiting.\n");
		return -1;
	}

#if BUILD_WIN32 == 1
	WSACleanup();
#endif
	return 0;
}

void Application::PrintProgramTitle()
{
	Utils::Log("\nXashMS started: platform {}, architecture {}, commit {}/{}\n",
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

	m_argsParser.add_argument("-u", "--unbuffered")
		.help("force stdout and stderr streams to be unbuffered")
		.flag();

	m_argsParser.add_argument("-cfg", "--config-file")
		.help("configuration file path")
		.default_value("config.json");
}

void Application::InitializeSocketInet()
{
	NetAddress address(NetAddress::AddressFamily::IPv4);
	std::string addressString = m_argsParser.get<std::string>("--ip").c_str();
	uint16_t portNumber = m_argsParser.get<int>("--port");

	if (address.FromString(addressString.c_str(), portNumber))
	{
		try {
			m_socketInet = std::make_shared<Socket>(AF_INET, SOCK_DGRAM, 0);
			m_socketInet->Bind(address);
			Utils::Log("Server IPv4 address: {}:{}\n", address.ToString(), address.GetPort());
		}
		catch (const std::exception &ex) {
			m_socketInet.reset();
			Utils::Log("Failed to initialize IPv4 socket: {}\n", ex.what());
		}
	}
	else {
		Utils::Log("Failed to parse IPv4 interface address\n");
	}
}

void Application::InitializeSocketInet6()
{
	NetAddress address(NetAddress::AddressFamily::IPv6);
	std::string addressString = m_argsParser.get<std::string>("--ip6").c_str();
	uint16_t portNumber = m_argsParser.get<int>("--port");

	if (address.FromString(addressString.c_str(), portNumber)) 
	{
		try {
			m_socketInet6 = std::make_shared<Socket>(AF_INET6, SOCK_DGRAM, 0);
			m_socketInet6->Bind(address);
			Utils::Log("Server IPv6 address: [{}]:{}\n", address.ToString(), address.GetPort());
		}
		catch (const std::exception &ex) {
			m_socketInet6.reset();
			Utils::Log("Failed to initialize IPv6 socket: {}\n", ex.what());
		}
	}
	else {
		Utils::Log("Failed to parse IPv6 interface address\n");
	}
}
