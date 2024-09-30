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

#include "libevent_wrappers.h"
#include <stdexcept>

ev::EventBase::EventBase()
{
	m_address = event_base_new();
	if (!m_address) {
		throw std::runtime_error("failed to initialize libevent base");
	}
}

ev::EventBase::~EventBase()
{
	event_base_free(m_address);
}

int ev::EventBase::Dispatch()
{
	return event_base_dispatch(m_address);
}

int ev::EventBase::LoopExit(const timeval *delay)
{
	return event_base_loopexit(m_address, delay);
}

ev::Event::Event(ev::EventBase &base, evutil_socket_t fd, short events, event_callback_fn callback, void *callback_arg)
{
	m_address = event_new(base.m_address, fd, events, callback, callback_arg);
	if (!m_address) {
		throw std::runtime_error("failed to initialize libevent event instance");
	}
}

ev::Event::~Event()
{
	event_free(m_address);
}

void ev::Event::Add(const timeval *timeout)
{
	event_add(m_address, timeout);
}
