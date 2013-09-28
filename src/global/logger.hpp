
//         Copyright Eóin O'Callaghan 2006 - 2009.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef GLOBAL_LOGGER
#define GLOBAL_LOGGER

#include <string>
#include <sstream>
#include <fstream>

#include <boost/signals2.hpp>
#include <boost/function.hpp>

namespace hal 
{

template<class _string_type, class _stream_type>
class Logger
{
public:
	boost::signals::scoped_connection attach(boost::function<void (const _string_type& text)> fn)
	{
		return logger_.connect(fn);
	}
	
	Logger<_string_type, _stream_type>& operator<<(const _string_type& text)
	{
		logger_(text);
		return *this;
	}

	template<class _type>
	Logger<_string_type, _stream_type>& operator<<(_type type)
	{
		_stream_type temp;
		temp << type;
		logger_(temp.str());
		return *this;
	}
#ifdef _UNICODE
	friend Logger<std::wstring, std::wostringstream>& wlog();
#endif
	friend Logger<std::string, std::ostringstream>& log();
	
private:
	Logger() {}
	Logger(const Logger& rhs) {}
	
	boost::signal<void (const _string_type& text)> logger_;
};

#ifdef _UNICODE
Logger<std::wstring, std::wostringstream>& wlog();
#endif
Logger<std::string, std::ostringstream>& log();


// N.B. What follows is a very low level logger only intended for debugging purposes

#ifndef NDEBUG

class global_log_file
{
public:
	global_log_file(std::string file) :
		filename(file)
	{}
	
	void operator()(const std::wstring& text)
	{
		if (!wofs.is_open()) wofs.open(filename.c_str());			
		wofs << text;
		
		wofs.flush();
	}
	
	void operator()(const std::string& text)
	{
		if (!wofs.is_open()) wofs.open(filename.c_str());
		
		for (std::string::const_iterator i=text.begin(); i!=text.end(); ++i)
		{
			char narrow_char = *i;
			wchar_t wide_char = 0;
			
			*(reinterpret_cast<char*>(&wide_char)) = narrow_char;			
			wofs << wide_char;
		}
		
		wofs.flush();
	}
	
private:	
	std::wofstream wofs;
	std::string filename;
	
};

inline global_log_file& log_file_singleton_dont_call_directly() 
{
	static global_log_file log("debug_log.txt");
	return log;
}

inline void log_file(const std::string& text)
{
	log_file_singleton_dont_call_directly()(text);
}

inline void log_file(const std::wstring& text)
{
	log_file_singleton_dont_call_directly()(text);
}

#else

inline void log_file(const std::string& text)
{}

inline void log_file(const std::wstring& text)
{}

#endif

} // namespace hal

#endif // GLOBAL_LOGGER
