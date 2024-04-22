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
#include "socket.h"
#include "net_address.h"
#include "infostring_data.h"
#include "server_list.h"
#include <unordered_map>
#include <string>

class RequestHandler
{
public:
	RequestHandler(ServerList &serverList);
	void HandlePacket(Socket &socket, const NetAddress &sourceAddr);

private:
	void ProcessClientQuery(Socket &socket, const NetAddress &sourceAddr);
	void ProcessChallengeRequest(Socket &socket, const NetAddress &sourceAddr);
	void ProcessAddServerRequest(Socket &socket, const NetAddress &sourceAddr);
	void SendFakeServerInfo(Socket &socket, const NetAddress &dest, InfostringData &data);
	void SendNatBypassNotify(Socket &socket, const NetAddress &dest, const NetAddress &client);
	bool ValidateClientQueryInfostring(const InfostringData &data);
	bool ValidateAddServerInfostring(const InfostringData &data);

	ServerList &m_serverList;
};
