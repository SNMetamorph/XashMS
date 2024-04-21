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
#include "socket.h"
#include "request_handler.h"
#include "server_list.h"
#include <memory>

class EventLoop
{
public:
	EventLoop(std::shared_ptr<Socket> socket_ipv4, std::shared_ptr<Socket> socket_ipv6);
	~EventLoop();

	void Run();

	struct LibeventImpl;
	std::shared_ptr<Socket> m_socket_ipv4;
	std::shared_ptr<Socket> m_socket_ipv6;
	std::unique_ptr<RequestHandler> m_packetHandler;
	std::unique_ptr<ServerList> m_serverList;
	std::unique_ptr<LibeventImpl> m_libeventImpl;
};
