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
#include "request_handler.h"
#include "server_list.h"
#include <event2/event.h>
#include <event2/event_struct.h>
#include <event2/util.h>
#include <iostream>
#include <stdexcept>
#include <csignal>

struct EventLoop::Impl
{
	Impl() : 
		eventBase(nullptr), 
		eventReceiveIPv4(nullptr), 
		eventReceiveIPv6(nullptr),
		eventCleanupTimer(nullptr),
		eventSigtermSignal(nullptr),
		eventSigintSignal(nullptr) {};

	void EventRecvIPv4();
	void EventRecvIPv6();
	void EventCleanupTimer();

	event_base *eventBase;
	event *eventReceiveIPv4;
	event *eventReceiveIPv6;
	event *eventCleanupTimer;
	event *eventSigtermSignal;
	event *eventSigintSignal;

	std::shared_ptr<Socket> socketIPv4;
	std::shared_ptr<Socket> socketIPv6;
	std::unique_ptr<RequestHandler> packetHandler;
	std::unique_ptr<ServerList> serverList;
};

EventLoop::EventLoop(std::shared_ptr<Socket> socketIPv4, std::shared_ptr<Socket> socketIPv6)
{
	m_impl = std::make_unique<Impl>();
	m_impl->socketIPv4 = socketIPv4;
	m_impl->socketIPv6 = socketIPv6;
	m_impl->serverList = std::make_unique<ServerList>();
	m_impl->packetHandler = std::make_unique<RequestHandler>(*m_impl->serverList);

	m_impl->eventBase = event_base_new();
	if (!m_impl->eventBase) {
		throw std::runtime_error("failed to initialize libevent base");
	} 
	else {
		evutil_secure_rng_init();
	}

	if (m_impl->socketIPv4)
	{
		auto ipv4RecvCallback = [](evutil_socket_t fd, short event, void *arg) {
			EventLoop *eventLoop = reinterpret_cast<EventLoop*>(arg);
			eventLoop->m_impl->EventRecvIPv4();
		};

		m_impl->eventReceiveIPv4 = event_new(
			m_impl->eventBase,
			m_impl->socketIPv4->GetDescriptor(),
			EV_READ | EV_PERSIST,
			ipv4RecvCallback,
			this
		);
		event_add(m_impl->eventReceiveIPv4, NULL);
	}

	if (m_impl->socketIPv6)
	{
		auto ipv6RecvCallback = [](evutil_socket_t fd, short event, void *arg) {
			EventLoop *eventLoop = reinterpret_cast<EventLoop*>(arg);
			eventLoop->m_impl->EventRecvIPv6();
		};

		m_impl->eventReceiveIPv6 = event_new(
			m_impl->eventBase,
			m_impl->socketIPv6->GetDescriptor(),
			EV_READ | EV_PERSIST,
			ipv6RecvCallback,
			this
		);
		event_add(m_impl->eventReceiveIPv6, NULL);
	}

	auto cleanupTimerCallback = [](evutil_socket_t fd, short event, void *arg) {
		EventLoop *eventLoop = reinterpret_cast<EventLoop*>(arg);
		eventLoop->m_impl->EventCleanupTimer();
	};

	m_impl->eventCleanupTimer = event_new(
		m_impl->eventBase, 
		-1, 
		EV_PERSIST, 
		cleanupTimerCallback, 
		this
	);

	timeval timerInterval = { 10, 0 };
	event_add(m_impl->eventCleanupTimer, &timerInterval);

	auto signalCallback = [](evutil_socket_t fd, short event, void *arg) {
		EventLoop *eventLoop = reinterpret_cast<EventLoop*>(arg);
		event_base_loopexit(eventLoop->m_impl->eventBase, nullptr);
	};

	m_impl->eventSigtermSignal = event_new(
		m_impl->eventBase, 
		SIGTERM, 
		EV_PERSIST | EV_SIGNAL, 
		signalCallback, 
		this
	);

	m_impl->eventSigintSignal = event_new(
		m_impl->eventBase, 
		SIGINT, 
		EV_PERSIST | EV_SIGNAL, 
		signalCallback, 
		this
	);

	event_add(m_impl->eventSigtermSignal, nullptr);
	event_add(m_impl->eventSigintSignal, nullptr);
}

EventLoop::~EventLoop()
{
	if (m_impl->eventReceiveIPv4) {
		event_free(m_impl->eventReceiveIPv4);
	}
	if (m_impl->eventReceiveIPv6) {
		event_free(m_impl->eventReceiveIPv6);	
	}
	if (m_impl->eventCleanupTimer) {
		event_free(m_impl->eventCleanupTimer);
	}
	if (m_impl->eventSigtermSignal) {
		event_free(m_impl->eventSigtermSignal);
	}
	if (m_impl->eventSigintSignal) {
		event_free(m_impl->eventSigintSignal);
	}
	if (m_impl->eventBase) {
		event_base_free(m_impl->eventBase);
	}

	m_impl->eventReceiveIPv4 = nullptr;
	m_impl->eventReceiveIPv6 = nullptr;
	m_impl->eventCleanupTimer = nullptr;
	m_impl->eventSigtermSignal = nullptr;
	m_impl->eventSigintSignal = nullptr;
	m_impl->eventBase = nullptr;
}

void EventLoop::Run()
{
	event_base_dispatch(m_impl->eventBase);
}

void EventLoop::Impl::EventRecvIPv4()
{
	NetAddress senderAddr = socketIPv4->RecvFrom();
	packetHandler->HandlePacket(*socketIPv4, senderAddr);
}

void EventLoop::Impl::EventRecvIPv6()
{
	NetAddress senderAddr = socketIPv6->RecvFrom();
	packetHandler->HandlePacket(*socketIPv6, senderAddr);
}

void EventLoop::Impl::EventCleanupTimer()
{
	serverList->CleanupStallServers();
}
