//
//  i_client.h
//  GameServerVK
//
//  Created by Veronika Kotckovich on 2021-05-30.
//

#ifndef i_client_h
#define i_client_h


#include "message.hpp"
#include "common.h"
#include "tsqueue.h"
#include "connection.h"

namespace server
{
	template <typename T>
	class client_interface
	{
	
	public:
	
		client_interface() : m_socket(m_context)
		{
			// initialize the socket with th io context, so it can do stuff
		}
	
		virtual ~client_interface()
		{
			//if client is destroyed try to disconect
			Disconnect();
		}
		
		// conct to server with hostname/ip-address and port
		bool Connect(const std::string& host, const uint16_t port)
		{
			try {
				//Resolve hostname/ip-address into tangiable physical address
				asio::ip::tcp::resolver resolver(m_context);
				asio::ip::tcp::resolver::results_type endpoints = resolver.resolve(host, std::to_string(port));

				// create connection
				m_connection = std::make_unique<connection<T>>(connection<T>::owner::client, m_context, asio::ip::tcp::socket(m_context), m_qMessagesIn);
				
				// Tell the connection object to connect to server
				m_connection->ConnectToServer(endpoints);
				
				// Start Conect thread
				thrContext = std::thread([this]() { m_context.run(); });
				
			} catch (std::exception& e) {
				std::cerr << "Client exception: " << e.what() << std::endl;
				return false;
			}
		
			return true;
		}
		
		// Disconect from the server
		void Disconnect()
		{
			if (IsConnected())
			{
				// disconect
				m_connection->Disconnect();
			}
		
			m_context.stop();
			if (thrContext.joinable())
			{
				thrContext.join();
			}
		
			// Destroy the connection
			m_connection.release();
		}
		
		// Checks if client connected to the server
		bool IsConnected()
		{
			if (m_connection)
			{
				return m_connection->IsConnected();
			}
			else
			{
				return false;
			}
		}
		
		// Retrive queue of messages from the server
		tsqueue<owned_message<T>>& Incoming()
		{
			return m_qMessagesIn;
		}
	
	// Send message to server
	void Send(const message<T>& msg)
	{
		if (IsConnected())
			m_connection->Send(msg);
	}
	
	protected:
		// asio context handles the data transfer
		asio::io_context m_context;
		// needs thread to execute its work commands
		std::thread thrContext;
		// this is the hardware srver connected to the server
		asio::ip::tcp::socket m_socket;
		// the client has a single instance of a "connection" object, which handles data transfer
		std::unique_ptr<connection<T>> m_connection;
		// vector of endpoints 
		std::vector<asio::ip::tcp::endpoint> m_endpoints;
	
	private:
		//This is thread safe queue of incoming messages from sever
		tsqueue<owned_message<T>> m_qMessagesIn;
	};
};
#endif /* i_client_h */
