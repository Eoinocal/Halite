//         Copyright Eóin O'Callaghan 2010 - 2010.
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

class poly_lock;
typedef boost::shared_ptr<poly_lock> poly_lock_ptr;

typedef boost::shared_ptr<boost::upgrade_to_unique_lock<boost::shared_mutex> > upgrade_to_unique_ptr;

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

class poly_lock
{
public:
	virtual ~poly_lock() {};

	virtual upgrade_to_unique_ptr upgrade_lock() = 0;
	virtual bool owns_lock() const = 0;
};

class poly_upgrade_to_unique_lock;

class poly_shared_lock : public poly_lock
{
public:
	poly_shared_lock(boost::shared_mutex& m) :
		p_(m)
	{}
		
	virtual upgrade_to_unique_ptr upgrade_lock()
	{
		throw invalid_poly_lock(L"Can't upgrade poly_shared_lock");
	}

	virtual bool owns_lock() const
	{
		return p_.owns_lock();
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
		
	virtual upgrade_to_unique_ptr upgrade_lock()
	{
		return upgrade_to_unique_ptr(new boost::upgrade_to_unique_lock<boost::shared_mutex>(p_));
	}

	virtual bool owns_lock() const
	{
		return p_.owns_lock();
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
		
	virtual upgrade_to_unique_ptr upgrade_lock()
	{
		return upgrade_to_unique_ptr();
	}

	virtual bool owns_lock() const
	{
		return p_.owns_lock();
	}

private:
	boost::unique_lock<boost::shared_mutex> p_;
};

class poly_upgrade_to_unique_lock
{
public:
	poly_upgrade_to_unique_lock(poly_lock_ptr p) :
		l_(p->upgrade_lock())
	{}

	bool owns_lock() const
	{
		return l_->owns_lock();
	}

private:
	upgrade_to_unique_ptr l_;
};

}
