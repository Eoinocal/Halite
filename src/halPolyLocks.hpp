//         Copyright Eóin O'Callaghan 2006 - 2009.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#if defined(HALTORRENT_PCH)
#	include "halPch.hpp"
#else
#	include "halTypes.hpp"
#endif

namespace hal {

class invalid_poly_lock : public std::exception
{
public:
	invalid_poly_lock(const std::wstring& s) :
		who_(s)
	{}
	
	virtual ~invalid_poly_lock() throw () {}

	std::wstring who() const throw ()
	{
		return who_;
	}       
	
private:
	std::wstring who_;	
};

class poly_lock;
typedef boost::shared_ptr<poly_lock> poly_lock_ptr;

class poly_lock_proxy
{
public:
	poly_lock_proxy()
	{}

	poly_lock_proxy(boost::upgrade_lock<boost::shared_mutex>& l) :
		l_(new boost::upgrade_to_unique_lock<boost::shared_mutex>(l))
	{}

private:
	boost::shared_ptr<boost::upgrade_to_unique_lock<boost::shared_mutex> > l_;
};

class poly_lock
{
public:
	virtual ~poly_lock();
	virtual poly_lock_proxy upgrade_lock() = 0;
};

class poly_shared_lock : public poly_lock
{
public:
	poly_shared_lock(boost::shared_mutex& m) :
		p_(m)
	{}
		
	virtual poly_lock_proxy upgrade_lock()
	{
		throw invalid_poly_lock(L"Can't upgrade poly_shared_lock");
	}

private:
	boost::shared_lock<boost::shared_mutex> p_;
};

class poly_upgrade_lock : public poly_lock
{
public:
	poly_upgrade_lock(boost::shared_mutex& m) :
		p_(m)
	{}
		
	virtual poly_lock_proxy upgrade_lock()
	{
		poly_lock_proxy(p_);
	}

private:
	boost::upgrade_lock<boost::shared_mutex> p_;
};

class poly_unique_lock : public poly_lock
{
public:
	poly_unique_lock(boost::shared_mutex& m) :
		p_(m)
	{}
		
	virtual poly_lock_proxy upgrade_lock()
	{
		poly_lock_proxy();
	}

private:
	boost::unique_lock<boost::shared_mutex> p_;
};

}
