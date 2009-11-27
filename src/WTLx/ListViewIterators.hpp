
//         Copyright Eóin O'Callaghan 2008 - 2009.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef LISTVIEW_ITERATORS_HPP_INCLUDED
#define LISTVIEW_ITERATORS_HPP_INCLUDED

#include <boost/iterator/filter_iterator.hpp>
#pragma warning (push)
#pragma warning (disable : 4244)
#	include <winstl/controls/listview_sequence.hpp>
#pragma warning (pop)

namespace WTLx
{

template<typename List>
class ListViewIterators
{
public:
	winstl::listview_sequence::const_iterator const_begin()
	{
		winstl::listview_sequence lv_seq(*(static_cast<List*>(this)));
		return lv_seq.begin();
	}

	winstl::listview_sequence::const_iterator const_end()
	{
		winstl::listview_sequence lv_seq(*(static_cast<List*>(this)));
		return lv_seq.end();
	}

	struct is_selected {
	bool operator()(const winstl::listview_sequence::sequence_value_type& v) 
	{ 
		return (v.state() & LVIS_SELECTED) != 0;
	} };

	typedef boost::filter_iterator<is_selected, 
			winstl::listview_sequence::const_iterator> is_selected_iterator;

	is_selected_iterator is_selected_begin()
	{
		winstl::listview_sequence lv_seq(*(static_cast<List*>(this)));
		return boost::make_filter_iterator<is_selected>(lv_seq.begin(), lv_seq.end());
	}

	is_selected_iterator is_selected_end()
	{
		winstl::listview_sequence lv_seq(*(static_cast<List*>(this)));
		return boost::make_filter_iterator<is_selected>(lv_seq.end(), lv_seq.end());
	}

	struct is_checked 
	{
		is_checked(ListViewIterators* t) :
			t_(t)
		{}
		bool operator()(const winstl::listview_sequence::sequence_value_type& v) 
		{ 
			return ((static_cast<const List*>(t_))->GetCheckState(v.index()) ? true : false);
		} 
	private:
		ListViewIterators* t_;
	};

	typedef boost::filter_iterator<is_checked, 
			winstl::listview_sequence::const_iterator> is_checked_iterator;

	is_checked_iterator is_checked_begin()
	{
		winstl::listview_sequence lv_seq(*(static_cast<List*>(this)));
		return boost::make_filter_iterator<is_checked>(is_checked(this), lv_seq.begin(), lv_seq.end());
	}

	is_checked_iterator is_checked_end()
	{
		winstl::listview_sequence lv_seq(*(static_cast<List*>(this)));
		return boost::make_filter_iterator<is_checked>(is_checked(this), lv_seq.end(), lv_seq.end());
	}

	typedef winstl::listview_sequence::sequence_value_type list_value_type;
};

}

#endif // LISTVIEW_ITERATORS_HPP_INCLUDED
