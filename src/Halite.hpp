
//         Copyright Eóin O'Callaghan 2006 - 2009.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once

using std::string;
using std::wstring;

using boost::lexical_cast;
using boost::numeric_cast;
using boost::array;
using boost::bind;
using boost::thread;
using boost::shared_ptr;
using boost::scoped_ptr;
using boost::filesystem::path;
using boost::filesystem::wpath;
using boost::noncopyable;

template<class Archive>
void serialize(Archive& ar, WTL::CRect& rect, const unsigned int version)
{
	ar & BOOST_SERIALIZATION_NVP(rect.top);
	ar & BOOST_SERIALIZATION_NVP(rect.bottom);
	ar & BOOST_SERIALIZATION_NVP(rect.left);
	ar & BOOST_SERIALIZATION_NVP(rect.right);
}

namespace hal
{	
	namespace fs = boost::filesystem;
	namespace pt = boost::posix_time;
	namespace xp = boost::xpressive;
	namespace sl = boost::serialization;

	using std::pair;
	using std::make_pair;
	
	using boost::tuple;

	typedef boost::int64_t size_type;
}


#include "global/wtl_app.hpp"
#include "global/string_conv.hpp"

#include "halIni.hpp"
#include "halEvent.hpp"
#include "halTorrent.hpp"

template<class T>
class RedrawLock
{
public:
	RedrawLock(T& window) :
		window_(window)
	{
		window_.SetRedraw(false);
	}
	
	~RedrawLock()
	{
		unlock();
	}
	
	void unlock()
	{
		window_.SetRedraw(true);
		window_.InvalidateRect(NULL, true);
	}
	
private:
	T& window_;
};
	
class Halite :
	public hal::IniBase<Halite>,
	private boost::noncopyable
{
public:
	Halite();
	
	friend class boost::serialization::access;
	template<class Archive>
	void serialize(Archive& ar, const unsigned int version)
	{	
		using boost::serialization::make_nvp;

		switch (version)
		{
		case 4:
		ar	& make_nvp("one_inst", oneInst)
			& make_nvp("show_message", showMessage_)
			& make_nvp("log_debug", logDebug_)
			& make_nvp("log_list_length", logListLen_)
			& make_nvp("log_to_file", logToFile_)
			& make_nvp("lang_dll", dll_);
		break;

		case 3:
		case 2:
		ar	& make_nvp("dll_", dll_);
		case 1:
		ar	& make_nvp("logListLen_", logListLen_);
		case 0:
		ar	& make_nvp("oneInst", oneInst)
			& make_nvp("logDebug_", logDebug_)
			& make_nvp("showMessage", showMessage_)
			& make_nvp("logToFile_", logToFile_);
		}
	}	
	
	bool logToFile() { return logToFile_; }
	bool logDebug() { return logDebug_; }
	const std::wstring& dll() { return dll_; }
	const int logListLen() { return static_cast<int>(logListLen_); }
	bool showMessage() { return showMessage_; }
	
	friend class GeneralOptions;
	friend class GlobalOptions;
	friend class PortOptions;
	friend class SplashDialog;
	friend class AdvDebugDialog;
	friend int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow);
	
private:
	std::wstring dll_;
	bool oneInst;
	bool logDebug_;
	bool showMessage_;
	bool logToFile_;
	size_t logListLen_;
};

Halite& halite();

BOOST_CLASS_VERSION(Halite, 4)
