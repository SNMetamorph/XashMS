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
#include <event2/event.h>
#include <event2/event_struct.h>
#include <event2/util.h>

namespace ev
{
	class EventBase
	{
	public:
		friend class Event;

		EventBase();
		~EventBase();
		int Dispatch();
		int LoopExit(const timeval *delay = nullptr);

	private:
		event_base *m_address;
	};

	class Event
	{
	public:
		Event(EventBase &base, evutil_socket_t fd, short events, event_callback_fn callback, void *callback_arg);
		~Event();
		void Add(const timeval *timeout = nullptr);
	private:
		event *m_address;
	};
};
