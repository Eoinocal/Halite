
#include <string>

#include <boost/thread/thread.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/recursive_mutex.hpp>
#include <boost/thread/xtime.hpp>
#include <boost/thread/condition.hpp>
#include <boost/thread/tss.hpp>
#include <boost/format.hpp>

#include "halXmlRpc.hpp"
#include "halTorrent.hpp"

namespace remote
{
	XmlRpcServer Server;
	static int Port;
	
	class GetTorrentDetails : public XmlRpcServerMethod
	{
	public:
		GetTorrentDetails(XmlRpcServer* s) : 
			XmlRpcServerMethod("getTorrentDetails", s) 
		{}
	
		void execute(XmlRpcValue& params, XmlRpcValue& result)
		{
			int nArgs = params.size();
			
			halite::torrentBriefDetails tbd = halite::getTorrents();
			if (tbd) 
			{				
				for(size_t i=0; i<tbd->size(); i++) 
				{
					wstring details = (wformat(L"%1$.2f%%, (D-U) %2$.2f-%3$.2f kb/s")
						% ((*tbd)[i].completion * 100)
						% ((*tbd)[i].speed.first/1024)
						% ((*tbd)[i].speed.second/1024)
						).str();
						
					result[i][0] = halite::wcstombs((*tbd)[i].filename);
					result[i][1] = halite::wcstombs(details);
				}
			}
		}
	
		std::string help() { return std::string("Get Torrent Details"); }
	
	} getTorrentDetails(&Server);
	
	class GetPausedTorrents : public XmlRpcServerMethod
	{
	public:
		GetPausedTorrents(XmlRpcServer* s) : 
			XmlRpcServerMethod("getPausedTorrents", s) 
		{}
	
		void execute(XmlRpcValue& params, XmlRpcValue& result)
		{
			int nArgs = params.size();
			
			halite::torrentBriefDetails tbd = halite::getTorrents();
			if (tbd) 
			{		
				size_t j = 0;				
				for(size_t i=0; i<tbd->size(); i++) 
				{
					if ((*tbd)[i].status == L"Paused")					
						result[j++] = halite::wcstombs((*tbd)[i].filename);
				}
			}
		}
	
		std::string help() { return std::string("Get Paused Torrents"); }
	
	} getPausedTorrents(&Server);
	
	class GetActiveTorrents : public XmlRpcServerMethod
	{
	public:
		GetActiveTorrents(XmlRpcServer* s) : 
			XmlRpcServerMethod("getActiveTorrents", s) 
		{}
	
		void execute(XmlRpcValue& params, XmlRpcValue& result)
		{
			int nArgs = params.size();
			
			halite::torrentBriefDetails tbd = halite::getTorrents();
			if (tbd) 
			{		
				size_t j = 0;				
				for(size_t i=0; i<tbd->size(); i++) 
				{
					if ((*tbd)[i].status != L"Paused")					
						result[j++] = halite::wcstombs((*tbd)[i].filename);
				}
			}
		}
	
		std::string help() { return std::string("Get Active Torrents"); }
	
	} getActiveTorrents(&Server);
	
	class ResumeTorrent : public XmlRpcServerMethod
	{
	public:
		ResumeTorrent(XmlRpcServer* s) : 
			XmlRpcServerMethod("resumeTorrent", s) 
		{}
	
		void execute(XmlRpcValue& params, XmlRpcValue& result)
		{
			int nArgs = params.size();
			if (nArgs >= 2)
			{
				string filename = params[1];
				halite::resumeTorrent(halite::mbstowcs(filename));
			}			
		}
	
		std::string help() { return std::string("Resume Torrent"); }
	
	} resumeTorrent(&Server);
	
	class PauseTorrent : public XmlRpcServerMethod
	{
	public:
		PauseTorrent(XmlRpcServer* s) : 
			XmlRpcServerMethod("pauseTorrent", s) 
		{}
	
		void execute(XmlRpcValue& params, XmlRpcValue& result)
		{
			int nArgs = params.size();
			if (nArgs >= 2)
			{
				string filename = params[1];
				halite::pauseTorrent(halite::mbstowcs(filename));
			}			
		}
	
		std::string help() { return std::string("Pause Torrent"); }
	
	} pauseTorrent(&Server);
	
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