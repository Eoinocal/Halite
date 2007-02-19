
#ifndef GLOBAL_LOGGER
#define GLOBAL_LOGGER

#include <string>
#include <sstream>

#include <boost/signals.hpp>
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

} // namespace hal

#endif // GLOBAL_LOGGER
