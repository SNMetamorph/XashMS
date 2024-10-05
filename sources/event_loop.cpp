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
public:
	Impl(std::shared_ptr<Socket> socketInet, std::shared_ptr<Socket> socketInet6);

	void Run();
	void RecvInetCallback();
	void RecvInet6Callback();
	void CleanupTimerCallback();

private:
	void InitInetSocketEvent();
	void InitInet6SocketEvent();
	void InitCleanupTimerEvent();
	void InitSignalsEvents();

	std::shared_ptr<Socket> m_socketInet;
	std::shared_ptr<Socket> m_socketInet6;
	std::unique_ptr<ServerList> m_serverList;
	std::unique_ptr<RequestHandler> m_requestHandler;
	std::unique_ptr<ev::EventBase> m_eventBase;
	std::unique_ptr<ev::Event> m_receivePacketInetEvent;
	std::unique_ptr<ev::Event> m_receivePacketInet6Event;
	std::unique_ptr<ev::Event> m_cleanupTimerEvent;
	std::unique_ptr<ev::Event> m_sigtermSignalEvent;
	std::unique_ptr<ev::Event> m_sigintSignalEvent;
};

EventLoop::Impl::Impl(std::shared_ptr<Socket> socketInet, std::shared_ptr<Socket> socketInet6) :
	m_socketInet(socketInet),
	m_socketInet6(socketInet6),
	m_serverList(std::make_unique<ServerList>()),
	m_requestHandler(std::make_unique<RequestHandler>(*m_serverList)),
	m_eventBase(std::make_unique<ev::EventBase>())
{
	evutil_secure_rng_init();
	if (socketInet) {
		InitInetSocketEvent();
	}
	if (socketInet6) {
		InitInet6SocketEvent();
	}
	InitCleanupTimerEvent();
	InitSignalsEvents();
}

EventLoop::EventLoop(std::shared_ptr<Socket> socketInet, std::shared_ptr<Socket> socketInet6)
{
	m_impl = std::make_unique<Impl>(socketInet, socketInet6);
}

EventLoop::~EventLoop()
{
}

void EventLoop::Run()
{
	m_impl->Run();
}

void EventLoop::Impl::Run()
{
	m_eventBase->Dispatch();
}

void EventLoop::Impl::InitInetSocketEvent()
{
	auto inetRecvCallback = [](evutil_socket_t fd, short event, void *arg) {
		EventLoop::Impl *impl = reinterpret_cast<EventLoop::Impl*>(arg);
		impl->RecvInetCallback();
	};

	m_receivePacketInetEvent = std::make_unique<ev::Event>(
		*m_eventBase, 
		m_socketInet->GetDescriptor(),
		EV_READ | EV_PERSIST,
		inetRecvCallback,
		this
	);
	m_receivePacketInetEvent->Add();
}

void EventLoop::Impl::InitInet6SocketEvent()
{
	auto inet6RecvCallback = [](evutil_socket_t fd, short event, void *arg) {
		EventLoop::Impl *impl = reinterpret_cast<EventLoop::Impl*>(arg);
		impl->RecvInet6Callback();
	};

	m_receivePacketInet6Event = std::make_unique<ev::Event>(
		*m_eventBase,
		m_socketInet6->GetDescriptor(),
		EV_READ | EV_PERSIST,
		inet6RecvCallback,
		this
	);
	m_receivePacketInet6Event->Add();
}

void EventLoop::Impl::InitCleanupTimerEvent()
{
	auto cleanupTimerCallback = [](evutil_socket_t fd, short event, void *arg) {
		EventLoop::Impl *impl = reinterpret_cast<EventLoop::Impl*>(arg);
		impl->CleanupTimerCallback();
	};

	timeval timerInterval = { 10, 0 };
	m_cleanupTimerEvent = std::make_unique<ev::Event>(
		*m_eventBase, 
		-1, 
		EV_PERSIST, 
		cleanupTimerCallback, 
		this
	);
	m_cleanupTimerEvent->Add(&timerInterval);
}

void EventLoop::Impl::InitSignalsEvents()
{
	auto signalCallback = [](evutil_socket_t fd, short event, void *arg) {
		EventLoop::Impl *impl = reinterpret_cast<EventLoop::Impl*>(arg);
		impl->m_eventBase->LoopExit();
	};

	m_sigtermSignalEvent = std::make_unique<ev::Event>(
		*m_eventBase, 
		SIGTERM, 
		EV_PERSIST | EV_SIGNAL, 
		signalCallback, 
		this
	);

	m_sigintSignalEvent = std::make_unique<ev::Event>(
		*m_eventBase, 
		SIGINT, 
		EV_PERSIST | EV_SIGNAL, 
		signalCallback, 
		this
	);

	m_sigtermSignalEvent->Add();
	m_sigintSignalEvent->Add();
}

void EventLoop::Impl::RecvInetCallback()
{
	NetAddress senderAddr = m_socketInet->RecvFrom();
	m_requestHandler->HandlePacket(*m_socketInet, senderAddr);
}

void EventLoop::Impl::RecvInet6Callback()
{
	NetAddress senderAddr = m_socketInet6->RecvFrom();
	m_requestHandler->HandlePacket(*m_socketInet6, senderAddr);
}

void EventLoop::Impl::CleanupTimerCallback()
{
	m_serverList->CleanupStallServers();
}
