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

#include "admin_challenge_response.h"
#include <cstring>

AdminChallengeResponse::AdminChallengeResponse(uint32_t masterChallenge, uint32_t hashChallenge) :
	m_masterChallenge(masterChallenge),
	m_hashChallenge(hashChallenge)
{
}

void AdminChallengeResponse::Serialize(BinaryOutputStream &stream) const
{
	stream.WriteBytes(Header, std::strlen(AdminChallengeResponse::Header));
	stream.Write<uint32_t>(m_masterChallenge);
	stream.Write<uint32_t>(m_hashChallenge);
}
