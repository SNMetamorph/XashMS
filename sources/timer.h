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
#include <utility>

class Timer
{
public:
	Timer();
	void Reset();
	void SetInterval(double interval);
	bool CycleElapsed() const;
	bool IntervalElapsed(double interval) const;

private:
	double m_interval;
	double m_timePoint;
};

template<class T> 
class Expirable
{
public:
	Expirable(const T& object) : m_value(object) { m_expirationTimer.Reset(); }
	Expirable(T&& object) : m_value(std::move(object)) { m_expirationTimer.Reset(); }

	T& GetValue() { return m_value; }
	const T& GetValue() const { return m_value; }
	bool Expired(double interval) const { return m_expirationTimer.IntervalElapsed(interval); }

private:
	T m_value;
	Timer m_expirationTimer;
};
