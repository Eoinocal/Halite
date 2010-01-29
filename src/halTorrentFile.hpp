
//         Copyright Eóin O'Callaghan 2010 - 2010.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "halTorrentDefines.hpp"
#include "halTypes.hpp"

namespace hal 
{

namespace libt = libtorrent;
namespace sc = boost::statechart;
namespace mpl = boost::mpl;

class torrent_file
{
public:
	torrent_file()
	{}

	torrent_file(const wstring& on, const wstring& cn) :
		original_name_(on),
		current_name_(cn)
	{}

	void change_filename(const fs::wpath& fn)
	{
		completed_name_ = fn;
	}

private:
	fs::wpath original_name_;
	fs::wpath completed_name_;

	fs::wpath current_name_;

	int priority_;
};


} // namespace hal