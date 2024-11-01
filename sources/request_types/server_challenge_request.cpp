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

#include "server_challenge_request.h"

std::optional<ServerChallengeRequest> ServerChallengeRequest::Parse(BinaryInputStream &stream)
{
	ServerChallengeRequest object;
	if (stream.GetBufferSize() == 6)
	{	
		stream.SkipBytes(2); // skip header
		object.m_clientChallenge = stream.Read<uint32_t>(); // read client challenge 
	}
	else {
		object.m_clientChallenge = std::nullopt;
	}
	return object;
}
