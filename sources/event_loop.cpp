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
#include "libevent_wrappers.h"
#include <event2/util.h>
#include <iostream>
#include <csignal>

struct EventLoop::Impl
{
	Impl() {};

	void EventRecvIPv4();
	void EventRecvIPv6();
	void EventCleanupTimer();

	std::unique_ptr<ev::EventBase> eventBase;
	std::unique_ptr<ev::Event> eventReceiveIPv4;
	std::unique_ptr<ev::Event> eventReceiveIPv6;
	std::unique_ptr<ev::Event> eventCleanupTimer;
	std::unique_ptr<ev::Event> eventSigtermSignal;
	std::unique_ptr<ev::Event> eventSigintSignal;

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
	m_impl->eventBase = std::make_unique<ev::EventBase>();
	evutil_secure_rng_init();

	if (m_impl->socketIPv4)
	{
		auto ipv4RecvCallback = [](evutil_socket_t fd, short event, void *arg) {
			EventLoop *eventLoop = reinterpret_cast<EventLoop*>(arg);
			eventLoop->m_impl->EventRecvIPv4();
		};

		m_impl->eventReceiveIPv4 = std::make_unique<ev::Event>(
			*m_impl->eventBase, 
			m_impl->socketIPv4->GetDescriptor(),
			EV_READ | EV_PERSIST,
			ipv4RecvCallback,
			this
		);
		m_impl->eventReceiveIPv4->Add();
	}

	if (m_impl->socketIPv6)
	{
		auto ipv6RecvCallback = [](evutil_socket_t fd, short event, void *arg) {
			EventLoop *eventLoop = reinterpret_cast<EventLoop*>(arg);
			eventLoop->m_impl->EventRecvIPv6();
		};

		m_impl->eventReceiveIPv6 = std::make_unique<ev::Event>(
			*m_impl->eventBase,
			m_impl->socketIPv6->GetDescriptor(),
			EV_READ | EV_PERSIST,
			ipv6RecvCallback,
			this
		);
		m_impl->eventReceiveIPv6->Add();
	}

	auto cleanupTimerCallback = [](evutil_socket_t fd, short event, void *arg) {
		EventLoop *eventLoop = reinterpret_cast<EventLoop*>(arg);
		eventLoop->m_impl->EventCleanupTimer();
	};

	timeval timerInterval = { 10, 0 };
	m_impl->eventCleanupTimer = std::make_unique<ev::Event>(
		*m_impl->eventBase, 
		-1, 
		EV_PERSIST, 
		cleanupTimerCallback, 
		this
	);
	m_impl->eventCleanupTimer->Add(&timerInterval);

	auto signalCallback = [](evutil_socket_t fd, short event, void *arg) {
		EventLoop *eventLoop = reinterpret_cast<EventLoop*>(arg);
		eventLoop->m_impl->eventBase->LoopExit();
	};

	m_impl->eventSigtermSignal = std::make_unique<ev::Event>(
		*m_impl->eventBase, 
		SIGTERM, 
		EV_PERSIST | EV_SIGNAL, 
		signalCallback, 
		this
	);

	m_impl->eventSigintSignal = std::make_unique<ev::Event>(
		*m_impl->eventBase, 
		SIGINT, 
		EV_PERSIST | EV_SIGNAL, 
		signalCallback, 
		this
	);

	m_impl->eventSigtermSignal->Add();
	m_impl->eventSigintSignal->Add();
}

EventLoop::~EventLoop()
{
}

void EventLoop::Run()
{
	m_impl->eventBase->Dispatch();
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
