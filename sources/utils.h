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
#include <fmt/core.h>
#include <cstdio>
#include <vector>
#include <string_view>

namespace Utils
{
	std::vector<std::string_view> Tokenize(std::string_view input, std::string_view token);

	template<typename... T> void Log(fmt::format_string<T...> fmt, T&&... args) 
	{
		fmt::print(fmt, std::forward<T>(args)...);
		std::fflush(stdout);
	}
}
