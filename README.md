# XashMS
Asynchronous, cross-platform masterserver implementation for Xash3D FWGS engine. Written in C++17, based on libevent, supports IPv6. 

## What does this program do?
In context of Xash3D game engine, main purpose of masterserver is to provide list of dedicated servers to players, so they can join to some of these servers. 

To gather this list, dedicated servers periodically send "heartbeat" packet to master server, which is some kind of "keep-alive" packet and means that exact dedicated server still functioning and ready to accept connections from players, then masterserver adds this server address to list. 

In case if dedicated server stopped sending such packets (server was stopped, or Internet connection was lost), after some time masterserver will remove it from public servers list.

## Usage
- `--ip`, `-ip` - address of IPv4 interface, which will be listened for incoming packets
- `--ip6`, `-ip6` -  address of IPv6 interface, which will be listened for incoming packets
- `--port`, `-p` - number of port that will be used for incoming connections
