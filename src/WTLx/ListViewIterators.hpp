
//         Copyright Eóin O'Callaghan 2008 - 2009.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef LISTVIEW_ITERATORS_HPP_INCLUDED
#define LISTVIEW_ITERATORS_HPP_INCLUDED

#include <boost/iterator/iterator_facade.hpp>
#include <boost/iterator/filter_iterator.hpp>

namespace WTLx
{

template<typename List>
class ListViewIterators
{
public:
	template<typename Value, WORD Flag=LVNI_ALL>
	class list_item_iterator : public boost::iterator_facade<list_item_iterator<Value, Flag>, Value, boost::forward_traversal_tag>
	{
	public:
		list_item_iterator() : 
			list_(nullptr),
			index_({-1, -1})
		{}

		explicit list_item_iterator(List* l) : 
			list_(l),
			index_({-1, -1})
		{
			increment();
		}

		Value sub_item(int si) const 
		{ 
			LVITEM i;
			i.iItem = index_.iItem;
			i.iSubItem = si;
			i.iGroupId = index_.iGroup;
			i.mask = LVIF_STATE;

			list_->GetItem(&i);

			return i; 
		}

		std::wstring text(int si = 0) const
		{
			for (int nLen = 256; ; nLen *= 2)
			{
				wstring str(nLen, L'\0');

				if (list_->GetItemText(index_.iItem, si, const_cast<LPTSTR>(str.data()), static_cast<int>(str.capacity())) < nLen - 1)
					return str;
			}
		}

	 private:
		friend class boost::iterator_core_access;

		void increment() 
		{ 
			if (!list_->GetNextItemIndex(&index_, Flag))
				index_ = {-1, -1};
		}

		bool equal(const list_item_iterator& other) const
		{			
			return (index_.iItem == -1 && other.index_.iItem == -1) || 
				(list_ == other.list_ && index_.iItem == other.index_.iItem && index_.iGroup == other.index_.iGroup);
		}

		Value dereference() const 
		{ 
			return sub_item(0); 
		}
		
		List* list_;
		LVITEMINDEX index_;
	};

	list_item_iterator<const LVITEM> begin()
	{
		return list_item_iterator<const LVITEM>((static_cast<List*>(this)));
	}

	list_item_iterator<const LVITEM> end()
	{
		return list_item_iterator<const LVITEM>();
	}

	list_item_iterator<const LVITEM, LVNI_SELECTED> begin_selected()
	{
		return list_item_iterator<const LVITEM, LVNI_SELECTED>((static_cast<List*>(this)));
	}

	list_item_iterator<const LVITEM, LVNI_SELECTED> end_selected()
	{
		return list_item_iterator<const LVITEM, LVNI_SELECTED>();
	}

	typedef LVITEM list_value_type;
};

}

#endif // LISTVIEW_ITERATORS_HPP_INCLUDED
