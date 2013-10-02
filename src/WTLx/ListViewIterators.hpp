
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
	class item
	{
	public:
		item(List* l) :
			list_(l),
			item_({0})
		{}

		wstring text(int si = 0) const
		{		
			wstring str;

			for (int nLen = 256; ; nLen *= 2)
			{
				str = wstring (nLen, L'\0');

				if (list_->GetItemText(item_.iItem, si, const_cast<LPTSTR>(str.data()), static_cast<int>(str.capacity())) < nLen - 1)
					return str;
			}
		}

		int index() const
		{
			return item_.iItem;
		}

		UINT state(UINT mask) const
		{
			return list_->GetItemState(index(), mask);
		}
		
		void set_index(LVITEMINDEX index) const
		{
			item_.iItem = index.iItem;
			item_.iGroupId = index.iGroup;
		}

		List* list() const { return list_; }

	private:		
		List* list_;
		mutable LVITEM item_;
	};

	template<typename Value, WORD Flag=LVNI_ALL>
	class list_item_iterator : public boost::iterator_facade<list_item_iterator<Value, Flag>, Value, boost::forward_traversal_tag>
	{
	public:
		typedef Value value_type;

		list_item_iterator() : 
			index_({-1, -1}),
			item_(nullptr)
		{}

		explicit list_item_iterator(List* l) : 
			index_({-1, -1}),
			item_(l)
		{
			increment();
		}

	 private:
		friend class boost::iterator_core_access;

		void increment() 
		{ 
			if (!item_.list()->GetNextItemIndex(&index_, Flag))
				index_ = {-1, -1};
		}

		bool equal(const list_item_iterator& other) const
		{			
			return (index_.iItem == -1 && other.index_.iItem == -1) || 
				(item_.list() == other.item_.list() && index_.iItem == other.index_.iItem && index_.iGroup == other.index_.iGroup);
		}

		Value& dereference() const 
		{ 
			item_.set_index(index_);

			return item_; 
		}
		
		LVITEMINDEX index_;
		item item_;
	};

	list_item_iterator<const item> begin()
	{
		return list_item_iterator<const item>((static_cast<List*>(this)));
	}

	list_item_iterator<const item> end()
	{
		return list_item_iterator<const item>();
	}

	list_item_iterator<const item, LVNI_SELECTED> begin_selected()
	{
		return list_item_iterator<const item, LVNI_SELECTED>((static_cast<List*>(this)));
	}

	list_item_iterator<const item, LVNI_SELECTED> end_selected()
	{
		return list_item_iterator<const item, LVNI_SELECTED>();
	}

	typedef item list_value_type;
};

}

#endif // LISTVIEW_ITERATORS_HPP_INCLUDED
