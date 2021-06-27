//
//  server.h
//  GameServerVK
//
//  Created by Veronika Kotckovich on 2021-06-13.
//

#ifndef server_h
#define server_h
#include "common.h"
#include "tsqueue.h"
#include "message.hpp"
#include "connection.h"

namespace server
{
	template<typename T>
	class server_interface
	{
	public:
		server_interface(uint16_t port)
			: m_asioAcceptor(m_asioContext, asio::ip::tcp::endpoint(asio::ip::tcp::v4(), port))
		{}
	
		virtual ~server_interface()
		{
			Stop();
		}
	
		bool Start()
		{
			try
			{
				WaitForClientConnection();
				
				m_threadContext = std::thread([this]() { m_asioContext.run(); });
				
				
			}
			catch (std::exception& e)
			{
				// Something prohibited server form listening
				std::cerr << "[SERVER] Exception: " << e.what() << std::endl;
				return false;
			}
		
			std::cout << "[SERVER] Started" <<std::endl;
			return true;
		}
	
		void Stop()
		{
			// Request to conext to close
			m_asioContext.stop();
		
			// Tidy up the context thread
			if (m_threadContext.joinable()) m_threadContext.join();
		
			std::cout << "[SERVER] Stoped" <<std::endl;
		}
	
		// ASYNC - Instruct asio to wait for connection
		void WaitForClientConnection()
		{
			m_asioAcceptor.async_accept(
				[this](std::error_code ec, asio::ip::tcp::socket socket)
				{
					if (!ec)
					{
						std::cout << "[SERVER] New Connectiom " << socket.remote_endpoint() << std::endl;

						std::shared_ptr<connection<T>> newconn =
							std::make_shared<connection<T>>(connection<T>::owner::server,
							m_asioContext, std::move(socket), m_qMessagesIn);

						// Give the user server a chance to deny connction
						if(OnClientConnect(newconn))
						{
							m_deqConnections.push_back(std::move(newconn));
							m_deqConnections.back()->ConnectToClient(nIDCounter++);

							std::cout << "[" << m_deqConnections.back()->GetID() << "] Connection approved " << std::endl;
						}
						else
						{
							std::cout << "[------] Connection Denied " << std::endl;
						}
					}
					else
					{
						std::cout << "[SERVER] New connection error: " << ec.message() << std::endl;
					}
			
					// Prime the asio context with more work to do - again simply wait for another connection
					WaitForClientConnection();
				});
		}
	
		// Send a message to a specific client
		void MessageClient(std::shared_ptr<connection<T>> client, const message<T>& msg)
		{
			if (client && client->IsConnected())
			{
				client->Send(msg);
			}
			else
			{
				OnClientDisconnect(client);
				client.reset();
				m_deqConnections.erase(
				   std::remove(m_deqConnections.begin(), m_deqConnections.end(), client), m_deqConnections.end());
			}
		}
	
		// Send a message to all clients
		void MessageAllClients(const message<T>& msg, std::shared_ptr<connection<T>> pIgnoreClient = nullptr)
		{
			bool bInvalidClientExists = false;
		
			for(auto& client : m_deqConnections)
			{
				if (client && client->IsConnected() && client != pIgnoreClient)
				{
					client->Send(msg);
				}
				else
				{
					OnClientDisconnect(client);
					client.reset();
				}
			}
		
			if(bInvalidClientExists)
			{
				m_deqConnections.erase(
				   std::remove(m_deqConnections.begin(), m_deqConnections.end(), nullptr), m_deqConnections.end());
			}
		}
	
		void Update(size_t nMaxMessages = -1, bool bWait = false)
		{
			if (bWait) m_qMessagesIn.wait();
			
			size_t nMessagesCount = 0;
			while (nMessagesCount < nMaxMessages && !m_deqConnections.empty() && !m_qMessagesIn.isEmpty())
			{
				// Grab the firts message
				auto msg = m_qMessagesIn.pop_front();
			
				// Pass the message handler
				OnMessage(msg.remote, msg.msg);
			
				nMessagesCount++;
			}
		}
	
	protected:
		// Called when a clients connect, can refuse connection by return false
		virtual bool OnClientConnect(std::shared_ptr<connection<T>> client)
		{
			return false;
		}
	
		// Called when a client appears to have disconnected
		virtual void OnClientDisconnect(std::shared_ptr<connection<T>> client)
		{
		
		}
	
		// Called when a message arrives
		virtual void OnMessage(std::shared_ptr<connection<T>> client, message<T>& msg)
		{
		
		}
	
		// Thread safe queue for incoming messages packets
		tsqueue<owned_message<T>> m_qMessagesIn;
	
		// Container of active validated connections
		std::deque<std::shared_ptr<connection<T>>> m_deqConnections;
	
		// Order of declaration is important - it's also order if init-ion
		asio::io_context m_asioContext;
		std::thread m_threadContext;
	
		// These thinds need an asio context
		asio::ip::tcp::acceptor m_asioAcceptor;
	
		// Clients will be identified in the wider system via ID
		uint32_t nIDCounter = 10000;
	
	};
}

#endif /* server_h */
