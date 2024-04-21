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

#include "request_handler.h"
#include "master_protocol.h"
#include "binary_input_stream.h"
#include "binary_output_stream.h"
#include <fmt/core.h>

RequestHandler::RequestHandler(ServerList &serverList) :
	m_serverList(serverList)
{
}

void RequestHandler::HandlePacket(Socket &socket, const NetAddress &sourceAddr)
{
	auto &recvBuffer = socket.GetDataBuffer();
	if (recvBuffer.size() < 2) {
		return; // invalid size packet, ignore it
	}

	if (std::memcmp(recvBuffer.data(), MasterProtocol::clientQuery, 1) == 0) {
		ProcessClientQuery(socket, sourceAddr);
	}
	else if (std::memcmp(recvBuffer.data(), MasterProtocol::challengeRequest, 2) == 0) {
		ProcessChallengeRequest(socket, sourceAddr);
	}
	else if (std::memcmp(recvBuffer.data(), MasterProtocol::addServer, 2) == 0) {
		ProcessAddServerRequest(socket, sourceAddr);
	}
}

void RequestHandler::ProcessClientQuery(Socket &socket, const NetAddress &sourceAddr)
{
	auto &recvBuffer = socket.GetDataBuffer();
	BinaryInputStream request(recvBuffer.data(), recvBuffer.size());

	request.SkipBytes(2); // skip header
	if (request.EndOfFile()) {
		return; // invalid request
	}

	if (!request.SkipString()) { // placeholder addr, ignored
		return; 
	}

	std::string queryInfo;
	if (!request.ReadString(queryInfo)) {
		return; 
	}

	size_t slashPos = queryInfo.find_first_of('\\');
	if (slashPos == std::string::npos) {
		return; // invalid query info string
	}

	// parse query info string, and recompose it to key/value structure
	InfostringData infostringData(queryInfo);
	if (!ValidateClientQueryInfostring(infostringData)) {
		return; // invalid query info, ignore request
	}

	if (!infostringData.Contains("clver")) 
	{
		SendFakeServerInfo(socket, sourceAddr, infostringData);
		return;
	}

	std::vector<uint8_t> data;
	BinaryOutputStream response(data);
	response.WriteString(MasterProtocol::queryPacketHeader);

	if (infostringData.Contains("key")) 
	{
		try {
			uint32_t queryKey = std::stoi(infostringData.Get("key"), nullptr, 16);
			response.WriteByte(0x7F);
			response.Write<uint32_t>(queryKey);
			response.WriteByte(0x00);
		}
		catch (const std::exception &ex) {
			return; // invalid data in query key
		}
	}

	bool natBypass = infostringData.Get("nat").compare("0") != 0;
	for (const auto &entry : m_serverList.GetEntriesCollection()) 
	{
		auto &serverAddr = entry.GetAddress();
		if (!entry.ValidateChallenge())
			continue;

		if (serverAddr.GetAddressFamily() != sourceAddr.GetAddressFamily())
			continue;

		if (entry.NatBypassEnabled() != natBypass)
			continue;

		if (infostringData.Get("gamedir").compare(entry.GetGamedir()) != 0)
			continue;

		if (infostringData.Contains("protocol"))
		{
			uint32_t clientProtocol = std::atoi(infostringData.Get("protocol").c_str());
			if (entry.GetProtocolVersion() != clientProtocol) {
				continue;
			}
		}

		if (natBypass) 
		{
			std::vector<uint8_t> tempBuffer;
			BinaryOutputStream natBypassPacket(tempBuffer);
			std::string clientAddr = fmt::format("{}:{}", serverAddr.ToString(), serverAddr.GetPort());
			natBypassPacket.WriteString(MasterProtocol::natBypassPacketHeader);
			natBypassPacket.WriteBytes(clientAddr.c_str(), clientAddr.size());
			socket.SendTo(serverAddr, tempBuffer);
		}
		response.WriteNetAddress(serverAddr);
	}

	fmt::print("Client query: {}:{}, gamedir={}, clver={}, nat={}\n", 
		sourceAddr.ToString(), 
		sourceAddr.GetPort(), 
		infostringData.Get("gamedir"),
		infostringData.Get("clver"),
		infostringData.Get("nat"));

	// write null address as a end of message marker
	response.WriteByte(0x00, 6);
	socket.SendTo(sourceAddr, data);
}

void RequestHandler::ProcessChallengeRequest(Socket &socket, const NetAddress &sourceAddr)
{
	if (m_serverList.CountForAddress(sourceAddr) > ServerList::GetMaxServersForIP()) {
		return; // too much servers for this IP
	}

	auto &recvBuffer = socket.GetDataBuffer();
	if (!m_serverList.Contains(sourceAddr)) {
		m_serverList.Insert(sourceAddr);
	}

	auto entry = m_serverList.FindEntry(sourceAddr);
	if (entry->ChallengeDelay()) {
		return; 
	}

	std::vector<uint8_t> data;
	BinaryOutputStream response(data);
	BinaryInputStream request(recvBuffer.data(), recvBuffer.size());

	response.WriteString(MasterProtocol::challengePacketHeader);
	response.Write<uint32_t>(entry->GetChallenge());
	if (recvBuffer.size() == 6)
	{
		// write for second challenge 
		request.SkipBytes(2);
		response.Write<uint32_t>(request.Read<uint32_t>());
	}
	socket.SendTo(sourceAddr, data);
	entry->ResetChallengeDelay();
}

void RequestHandler::ProcessAddServerRequest(Socket &socket, const NetAddress &sourceAddr)
{
	auto &recvBuffer = socket.GetDataBuffer();
	BinaryInputStream request(recvBuffer.data(), recvBuffer.size());

	request.SkipBytes(2); // skip header bytes
	if (request.EndOfFile()) {
		return; // invalid request length
	}

	ServerEntry *entry = m_serverList.FindEntry(sourceAddr);
	if (!entry) 
	{
		fmt::print("Server skipped challenge request: {}:{}\n", sourceAddr.ToString(), sourceAddr.GetPort());
		return;
	}

	std::string infostring;
	InfostringData data;
	request.ReadString(infostring);
	data.Parse(infostring);

	if (!ValidateAddServerInfostring(data))
	{
		fmt::print("Invalid add server query infostring: {}:{}\n", sourceAddr.ToString(), sourceAddr.GetPort());
		return;
	}

	entry->Update(data); // TODO validate infostring fullness
	if (entry->ValidateChallenge()) 
	{
		entry->ResetTimeout();
		fmt::print(entry->IsApproved() ? "Updated " : "Added ");
		fmt::print("server: {}:{}, game={}/{}, protocol={}, players={}/{}/{}, version={}\n", 
			sourceAddr.ToString(), 
			sourceAddr.GetPort(), 
			entry->GetMapName(), 
			entry->GetGamedir(), 
			entry->GetProtocolVersion(),
			entry->GetPlayersCount(),
			entry->GetBotsCount(),
			entry->GetMaxPlayers(),
			entry->GetClientVersion());
		entry->Approve();
	}
	else {
		fmt::print("Incorrect challenge from {}:{}: value {}\n", sourceAddr.ToString(), sourceAddr.GetPort(), entry->GetChallenge());
	}
}

void RequestHandler::SendFakeServerInfo(Socket &socket, const NetAddress &dest, InfostringData &infostringData)
{
	std::vector<uint8_t> data;
	BinaryOutputStream response(data);
	auto sendServerInfo = [&](std::string message) {
		InfostringData infostring;
		infostring.Insert("host", message.c_str());
		infostring.Insert("map", "update");
		infostring.Insert("dm", "0");
		infostring.Insert("team", "0");
		infostring.Insert("coop", "0");
		infostring.Insert("numcl", "32");
		infostring.Insert("maxcl", "32");
		infostring.Insert("gamedir", infostringData.Get("gamedir").c_str());

		data.clear();
		response.WriteString(MasterProtocol::fakeServerInfoHeader);
		response.WriteString(infostring.ToString().c_str());
		socket.SendTo(dest, data);
	};

	sendServerInfo(u8"This version is not");
	sendServerInfo(u8"supported anymore");
	sendServerInfo(u8"Please update Xash3DFWGS");
	sendServerInfo(u8"From GooglePlay or GitHub");
	sendServerInfo(u8"Эта версия");
	sendServerInfo(u8"устарела");
	sendServerInfo(u8"Обновите Xash3DFWGS c");
	sendServerInfo(u8"GooglePlay или GitHub");
}

bool RequestHandler::ValidateClientQueryInfostring(const InfostringData &data)
{
	if (!data.Contains("gamedir")) {
		return false;
	}
	if (!data.Contains("nat")) {
		return false;
	}
	return true;
}

bool RequestHandler::ValidateAddServerInfostring(const InfostringData &data)
{
	if (!data.Contains("challenge")) {
		return false;
	}
	if (!data.Contains("protocol")) {
		return false;
	}
	if (!data.Contains("players")) {
		return false;
	}
	if (!data.Contains("max")) {
		return false;
	}
	if (!data.Contains("bots")) {
		return false;
	}
	if (!data.Contains("region")) {
		return false;
	}
	if (!data.Contains("gamedir")) {
		return false;
	}
	if (!data.Contains("map")) {
		return false;
	}
	if (!data.Contains("version")) {
		return false;
	}
	if (!data.Contains("os")) {
		return false;
	}
	if (!data.Contains("product")) {
		return false;
	}
	if (!data.Contains("type")) {
		return false;
	}
	if (!data.Contains("password")) {
		return false;
	}
	if (!data.Contains("secure")) {
		return false;
	}
	if (!data.Contains("lan")) {
		return false;
	}
	if (!data.Contains("nat")) {
		return false;
	}
	return true;
}
