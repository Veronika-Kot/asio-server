//
//  message.hpp
//  GameServerVK
//
//  Created by Veronika Kotckovich on 2021-05-15.
//

#ifndef message_hpp
#define message_hpp

#include <stdio.h>
#include "common.h"

namespace server
{

    ///Message header is sent at start of all messages. Template allows us to use enum to ensure the messages are valid
    template <typename T>
    struct message_header
    {
        T id{};
        uint32_t size = 0;
    };

    template <typename T>
    struct message
    {
        message_header<T> header{};
        std::vector<uint32_t> body;
        
        size_t size() const
        {
            return sizeof(message_header<T>) + body.size();
        }
        
        friend std::ostream& operator << (std::ostream& os, const message<T>& msg)
        {
            os << "ID" << int(msg.header.id) << " Size:"<< msg.header.size;
            return os;
        }
        
        ///Pushes any POD-like data into the message buffer
        template <typename DataType>
        friend message<T>& operator << (message<T>& msg, const DataType& data)
        {
            // Check that hte type of the data pushed is syrializable
            static_assert(std::is_standard_layout<DataType>::value, "Data is too complex to be pushed");
            
            //Cache current size if vector, as this will be the point we insert the data
            size_t i = msg.body.size();
            
            //Resize the vector by the size of the data being pushed
            msg.body.resize(msg.body.size() + sizeof(DataType));
			
			//Copy data into the newly allocated vector space
			std::memcpy(msg.body.data() + i, &data, sizeof(DataType));
		
			//Recalculate the message zise
			msg.header.size = msg.size();
		
			//Return the target message so it can be chained
			return msg;
        }
	
		///Taking any POD-like data from the message buffer
		template <typename DataType>
		friend message<T> &operator>> (message<T>& msg, DataType& data)
		{
			// Check that the type of the data pushed is syrializable
			static_assert(std::is_standard_layout<DataType>::value, "Data is too complex to be pushed");
			
			//Cache current the location towards the end of the vector where the pulled data starts
			size_t i = msg.body.size() - sizeof(DataType);
			
			//Copy data from the vector to user variable
			std::memcpy(&data, msg.body.data() + i, sizeof(DataType));
			
			//Making vector smaller
			msg.body.resize(i);
		
			//Recalculate the message zise
			msg.header.size = msg.size();
			
			//Return the target message so it can be chained
			return msg;
		}
    };
	
	// Forward declare
	template <typename T>
	class connection;
	
	template <typename T>
	struct owned_message
	{
		std::shared_ptr<connection<T>> remote = nullptr;
		message<T> msg;
	
		friend std::ostream& operator << (std::ostream& os, const owned_message<T>& msg)
		{
			os << msg.msg;
			return os;
		}
	};
}

#endif /* message_hpp */
