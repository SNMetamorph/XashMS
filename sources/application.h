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
#include <argparse/argparse.hpp>
#include "socket.h"
#include "event_loop.h"
#include "config_manager.h"
#include <memory>

class Application
{
public:
	Application();
	int Run(int argc, char *argv[]);

private:
	void PrintProgramTitle();
	void InitializeProgramArguments();
	void InitializeSocketInet();
	void InitializeSocketInet6();

	argparse::ArgumentParser m_argsParser;
	std::shared_ptr<Socket> m_socketInet;
	std::shared_ptr<Socket> m_socketInet6;
	std::shared_ptr<ConfigManager> m_configManager;
	std::unique_ptr<EventLoop> m_eventLoop;
};
