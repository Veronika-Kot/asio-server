//
//  TestServer.hpp
//  TestServer
//
//  Created by Veronika Kotckovich on 2021-06-14.
//

#ifndef TestServer_hpp
#define TestServer_hpp

#include <stdio.h>
#include <iostream>
#include "server_common.h"

enum class CustomMsgTypes : uint32_t
{
	ServerAccept,
	ServerDeny,
	ServerPing,
	MessageAll,
	ServerMessage
};

class TestServer : public server::server_interface<CustomMsgTypes>
{
public:
	TestServer(uint16_t nPort) : server::server_interface<CustomMsgTypes>(nPort)
	{
	
	}
protected:
	virtual bool OnClientConnect(std::shared_ptr<server::connection<CustomMsgTypes>> client)
	{
		server::message<CustomMsgTypes> msg;
		msg.header.id = CustomMsgTypes::ServerAccept;
		client->Send(msg);
		return true;
	}
	
	// Called when a client appears to have disconnected
	virtual void OnClientDisconnect(std::shared_ptr<server::connection<CustomMsgTypes>> client)
	{
		std::cout << "Removing client [" << client->GetID() << "]\n";
	}
	
	// Called when a message arrives
	virtual void OnMessage(std::shared_ptr<server::connection<CustomMsgTypes>> client, server::message<CustomMsgTypes>& msg)
	{
		switch (msg.header.id)
		{
			case CustomMsgTypes::ServerPing:
			{
				std::cout << "[" << client->GetID() << "]: Server Ping\n";
			
				// Simply bounce message back to client
				client->Send(msg);
			}
			break;
			
			case CustomMsgTypes::MessageAll:
			{
				std::cout << "[" << client->GetID() << "]: Message All\n";
			
				// Construct a new message and send it to all clients
				server::message<CustomMsgTypes> msg;
				msg.header.id = CustomMsgTypes::ServerMessage;
				msg << client->GetID();
				MessageAllClients(msg, client);
			}
			break;
		}
	}
};
#endif /* TestServer_hpp */
