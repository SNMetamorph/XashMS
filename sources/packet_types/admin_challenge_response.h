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
#include "binary_output_stream.h"
#include <stdint.h>

class AdminChallengeResponse
{
public:
	static constexpr const char *Header = "\xff\xff\xff\xff" "adminchallenge";

	AdminChallengeResponse(uint32_t masterChallenge, uint32_t hashChallenge) :
		m_masterChallenge(masterChallenge),
		m_hashChallenge(hashChallenge)
	{
	}

	void Serialize(BinaryOutputStream &stream) const;

private:
	uint32_t m_masterChallenge;
	uint32_t m_hashChallenge;
};
