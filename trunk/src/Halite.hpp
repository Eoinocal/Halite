
//         Copyright Eóin O'Callaghan 2006 - 2008.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once

using std::string;
using std::wstring;

using boost::lexical_cast;
using boost::array;
using boost::format;
using boost::wformat;
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

#define foreach BOOST_FOREACH

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
	Halite() :
		hal::IniBase<Halite>("globals/halite", "Halite"),
		oneInst(false),
#ifdef TORRENT_LOGGING
		logDebug_(true),
#else
		logDebug_(false),
#endif
		showMessage_(true),
		logToFile_(true),
		logListLen_(128),
		dll_(L"")
	{
		load();
	}
	
	friend class boost::serialization::access;
	template<class Archive>
	void serialize(Archive& ar, const unsigned int version)
	{	
        ar & BOOST_SERIALIZATION_NVP(oneInst);
		ar & BOOST_SERIALIZATION_NVP(logDebug_);
		ar & boost::serialization::make_nvp("showMessage", showMessage_);
		
		ar & BOOST_SERIALIZATION_NVP(logToFile_);
		if (version > 1)
			ar & BOOST_SERIALIZATION_NVP(logListLen_);
		if (version > 0)
			ar & BOOST_SERIALIZATION_NVP(dll_);
	}	
	
	bool logToFile() { return logToFile_; }
	bool logDebug() { return logDebug_; }
	const std::wstring& dll() { return dll_; }
	const int logListLen() { return logListLen_; }
	bool showMessage() { return showMessage_; }
	
	friend class GeneralOptions;
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

BOOST_CLASS_VERSION(Halite, 3)
