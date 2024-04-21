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

#include "event_loop.h"
#include <event2/event.h>
#include <event2/event_struct.h>
#include <event2/util.h>
#include <iostream>
#include <stdexcept>

struct EventLoop::LibeventImpl
{
	LibeventImpl() : 
		eventBase(nullptr), 
		eventReceiveIPv4(nullptr), 
		eventReceiveIPv6(nullptr),
		eventCleanupTimer(nullptr) {};

	event_base *eventBase;
	event *eventReceiveIPv4;
	event *eventReceiveIPv6;
	event *eventCleanupTimer;
};

EventLoop::EventLoop(std::shared_ptr<Socket> socket_ipv4, std::shared_ptr<Socket> socket_ipv6) :
	m_socket_ipv4(socket_ipv4),
	m_socket_ipv6(socket_ipv6)
{
	m_serverList = std::make_unique<ServerList>();
	m_packetHandler = std::make_unique<RequestHandler>(*m_serverList);
	m_libeventImpl = std::make_unique<LibeventImpl>();
	m_libeventImpl->eventBase = event_base_new();
	if (!m_libeventImpl->eventBase) {
		throw std::runtime_error("failed to initialize libevent base");
	} 
	else {
		evutil_secure_rng_init();
	}

	if (m_socket_ipv4)
	{
		auto ipv4RecvCallback = [](evutil_socket_t fd, short event, void *arg) {
			EventLoop *eventLoop = reinterpret_cast<EventLoop*>(arg);
			NetAddress senderAddr = eventLoop->m_socket_ipv4->RecvFrom();
			eventLoop->m_packetHandler->HandlePacket(*eventLoop->m_socket_ipv4, senderAddr);
		};

		m_libeventImpl->eventReceiveIPv4 = event_new(
			m_libeventImpl->eventBase,
			m_socket_ipv4->GetDescriptor(),
			EV_READ | EV_PERSIST,
			ipv4RecvCallback,
			this
		);
		event_add(m_libeventImpl->eventReceiveIPv4, NULL);
	}

	if (m_socket_ipv6)
	{
		auto ipv6RecvCallback = [](evutil_socket_t fd, short event, void *arg) {
			EventLoop *eventLoop = reinterpret_cast<EventLoop*>(arg);
			NetAddress senderAddr = eventLoop->m_socket_ipv6->RecvFrom();
			eventLoop->m_packetHandler->HandlePacket(*eventLoop->m_socket_ipv6, senderAddr);
		};

		m_libeventImpl->eventReceiveIPv6 = event_new(
			m_libeventImpl->eventBase,
			m_socket_ipv6->GetDescriptor(),
			EV_READ | EV_PERSIST,
			ipv6RecvCallback,
			this
		);
		event_add(m_libeventImpl->eventReceiveIPv6, NULL);
	}

	auto cleanupTimerCallback = [](evutil_socket_t fd, short event, void *arg) {
		EventLoop *eventLoop = reinterpret_cast<EventLoop*>(arg);
		eventLoop->m_serverList->CleanupStallServers();
	};

	m_libeventImpl->eventCleanupTimer = event_new(
		m_libeventImpl->eventBase, 
		-1, 
		EV_PERSIST, 
		cleanupTimerCallback, 
		this
	);

	timeval timerInterval = { 10, 0 };
	event_add(m_libeventImpl->eventCleanupTimer, &timerInterval);
}

EventLoop::~EventLoop()
{
	if (m_libeventImpl->eventReceiveIPv4) {
		event_free(m_libeventImpl->eventReceiveIPv4);
	}
	if (m_libeventImpl->eventReceiveIPv6) {
		event_free(m_libeventImpl->eventReceiveIPv6);	
	}
	if (m_libeventImpl->eventCleanupTimer) {
		event_free(m_libeventImpl->eventCleanupTimer);
	}
	if (m_libeventImpl->eventBase) {
		event_base_free(m_libeventImpl->eventBase);
	}

	m_libeventImpl->eventReceiveIPv4 = nullptr;
	m_libeventImpl->eventReceiveIPv6 = nullptr;
	m_libeventImpl->eventCleanupTimer = nullptr;
	m_libeventImpl->eventBase = nullptr;
}

void EventLoop::Run()
{
	event_base_dispatch(m_libeventImpl->eventBase);
}
