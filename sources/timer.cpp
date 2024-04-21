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

#include "timer.h"
#include <chrono>

Timer::Timer() :
	m_interval(1.0),
	m_lastTick(0.0)
{
}

void Timer::Reset()
{
	auto duration = std::chrono::steady_clock::now().time_since_epoch();
	m_lastTick = std::chrono::duration<double>(duration).count();
}

void Timer::SetInterval(double interval)
{
	m_interval = interval;
}

bool Timer::CycleElapsed() const
{
	auto duration = std::chrono::steady_clock::now().time_since_epoch();
	double currentTime = std::chrono::duration<double>(duration).count();
	return currentTime > (m_lastTick + m_interval);
}
