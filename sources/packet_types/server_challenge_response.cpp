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

#include "server_challenge_response.h"

ServerChallengeResponse::ServerChallengeResponse(uint32_t challenge, std::optional<uint32_t> clientChallenge) :
	m_challenge(challenge),
	m_clientChallenge(clientChallenge)
{
}

void ServerChallengeResponse::Serialize(BinaryOutputStream &stream) const
{
	stream.WriteString(ServerChallengeResponse::Header);
	stream.Write<uint32_t>(m_challenge);
	if (m_clientChallenge.has_value()) {
		stream.Write<uint32_t>(m_clientChallenge.value());
	}
}
