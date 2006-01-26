
#include <string>

#include <boost/thread/thread.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/recursive_mutex.hpp>
#include <boost/thread/xtime.hpp>
#include <boost/thread/condition.hpp>
#include <boost/thread/tss.hpp>

#include "halXmlRpc.hpp"

namespace remote
{
	XmlRpcServer Server;
	static int Port;
	
	class Hello : public XmlRpcServerMethod
	{
	public:
		Hello(XmlRpcServer* s) : XmlRpcServerMethod("Hello", s) 
		{}
	
		void execute(XmlRpcValue& params, XmlRpcValue& result)
		{
			int nArgs = params.size();
//			Log(wformat(L"%1%\r\n") % nArgs); 
			
			for (int i=0; i<nArgs; ++i)
			{
				switch (params[i].getType())
				{
					case XmlRpcValue::TypeInt:			
//						Log(wformat(L" %1%) Int:%2%\r\n") % i % int(params[i]));
					break;
					
					case XmlRpcValue::TypeString:			
//						Log(wformat(L" %1%) String\r\n") % i);
					break;
					
					case XmlRpcValue::TypeArray:			
//						Log(wformat(L" %1%) Array\r\n") % i);
					break;
					
				}
			}
			
			string resultstring = "Wayhaa";
			result[0] = resultstring;
			result[1] = 123;
			result[2][0] = 123;
			result[2][1] = "Hi";
		}
	
		std::string help() { return std::string("Say hello"); }
	
	} hello(&Server);
	
	class Close : public XmlRpcServerMethod
	{
	public:
		Close(XmlRpcServer* s) : XmlRpcServerMethod("close", s) 
		{}
	
		void execute(XmlRpcValue& params, XmlRpcValue& result)
		{}
	
		std::string help() { return std::string("Close Halite"); }
	
	} close(&Server);
	
	void RunServer()
	{
//		Log(wformat(L"Running Server.\r\n"));
		
		Server.bindAndListen(Port);
		
		// Enable introspection
		Server.enableIntrospection(true);
		
		// Wait for requests indefinitely
		Server.work(-1.0);
	  
//		Log(wformat(L"Shutting down server.\r\n"));	
	}
	
	bool initServer(int port)
	{
		try
		{
			Port = port;
			thread ServerThread(&RunServer);
			
			return true;
		}
		catch (...)
		{
			return false;
		}		
	}
	
	void exitServer()
	{
		Server.exit();
	}
}