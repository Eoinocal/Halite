#pragma once

#include <XmlRpc.h>

namespace remote 
{
	using namespace std;
	using namespace boost;
	using namespace XmlRpc;
	
	extern XmlRpcServer Server;
	void RunServer();
	bool initServer(int port);
	void exitServer();
}