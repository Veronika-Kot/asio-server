//
//  main.cpp
//  ClientVK
//
//  Created by Veronika Kotckovich on 2021-05-29.
//

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

class TestClient : public server::client_interface<CustomMsgTypes>
{
public:
	void PingServer()
	{
		server::message<CustomMsgTypes> msg;
		msg.header.id = CustomMsgTypes::ServerPing;
		
		// Caution with this...
		std::chrono::system_clock::time_point timeNow = std::chrono::system_clock::now();
		
		msg << timeNow;
		Send(msg);
	}
	
	void MessageAll()
	{
		server::message<CustomMsgTypes> msg;
		msg.header.id = CustomMsgTypes::MessageAll;
	
		//msg << "test";
		Send(msg);
	}
};


int main(int argc, const char * argv[]) {
	//server::message<CustomMsgTypes> msg;
	
	TestClient tc;
	tc.Connect("127.0.0.1", 6000);
	
	bool key[3] = { false, false, false };
	bool old_key[3] = { false, false, false };
	
	bool bQuit = false;
	while (!bQuit){
		if(tc.IsConnected())
		{
			std::string iKey = "";
			std::cin >> iKey;
		
			if(iKey == "1"){
				key[0] = !old_key[0];
			} else if (iKey == "2"){
				key[1] = !old_key[1];
			} else if (iKey == "3"){
				key[2] = !old_key[2];
			}
		
			if (key[0] && !old_key[0]) tc.MessageAll(); // tc.PingServer();
			if (key[1] && !old_key[1]) tc.MessageAll();
			if (key[2] && !old_key[2]) bQuit = true;
			
			for (int i = 0; i < 3; i++) old_key[i] = key[i];
			
			if (tc.IsConnected())
			{
				if (!tc.Incoming().isEmpty())
				{
					auto msg = tc.Incoming().pop_front().msg;
					
					switch (msg.header.id)
					{
						case CustomMsgTypes::ServerAccept:
						{
							// Server has responded to a ping request
							std::cout << "Server Accepted Connection\n";
						}
						break;
						
						case CustomMsgTypes::ServerPing:
						{
							// Server has responded to a ping request
							std::chrono::system_clock::time_point timeNow = std::chrono::system_clock::now();
							std::chrono::system_clock::time_point timeThen;
							msg >> timeThen;
							std::cout << "Ping: " << std::chrono::duration<double>(timeNow - timeThen).count() << "\n";
						}
						break;
						
						case CustomMsgTypes::ServerMessage:
						{
							// Server has responded to a ping request
							uint32_t clientID;
							msg >> clientID;
							std::cout << "Hello from [" << clientID << "]\n";
						}
						break;
					}
				}
			}
			else
			{
				std::cout << "Server Down\n";
				bQuit = true;
			}
		}
	}
	return 0;
}
