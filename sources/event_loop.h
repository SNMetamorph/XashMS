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
#include <memory>

class EventLoop
{
public:
	EventLoop(std::shared_ptr<Socket> socketIPv4, std::shared_ptr<Socket> socketIPv6);
	~EventLoop();
	void Run();

	struct Impl;
	std::unique_ptr<Impl> m_impl;
};
