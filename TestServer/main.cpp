//
//  main.cpp
//  TestServer
//
//  Created by Veronika Kotckovich on 2021-06-14.
//

#include <iostream>
#include "TestServer.hpp"

int main(int argc, const char * argv[]) {
	TestServer server(6000);
	server.Start();
	
	while(1) {
		server.Update();
	}
	
	std::cout << "Hello, World!\n";
	return 0;
}
