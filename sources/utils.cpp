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

#include "utils.h"

std::vector<std::string_view> Utils::Tokenize(std::string_view input, std::string_view token)
{
    size_t first = 0;
    std::vector<std::string_view> output;
    output.reserve(4);

    while (first < input.size())
    {
        size_t second = input.find_first_of(token, first);
        if (first != second) {
            output.emplace_back(input.substr(first, second - first));
        }
        if (second == std::string_view::npos) {
            break;
        }
        first = second + 1;
    }
    return output;
}
