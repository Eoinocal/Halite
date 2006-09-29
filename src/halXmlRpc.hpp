
#pragma once

#include <iostream>
#include <boost/smart_ptr.hpp>

namespace halite 
{
//	using namespace std;
//	using namespace boost;
//	using namespace http;
//	using namespace http::xmlrpc;
	
//	extern XmlRpcServer Server;
//	void RunServer();
	bool initServer(int port);
	void exitServer();
	
		
	class XmlRpc
	{
	public:		
		void bindHost(short port);
		void rebindHost(short port);
		void stopHost();
		
		friend XmlRpc& xmlRpc();
		
	private:
		XmlRpc();
		
		class XmlRpc_impl;
		boost::scoped_ptr<XmlRpc_impl> pimpl;
	};
	
	XmlRpc& xmlRpc();
}