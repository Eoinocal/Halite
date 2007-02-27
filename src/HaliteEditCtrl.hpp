
#include "stdAfx.hpp"
#include "global/string_conv.hpp"

template<typename T>
class CHalEditCtrl :
	public CWindowImpl<CHalEditCtrl<T>, CEdit>,
	private boost::noncopyable
{
protected:
	typedef CHalEditCtrl<T> thisClass;
	typedef CWindowImpl<CHalEditCtrl<T>, CEdit> baseClass;
	
public:
	CHalEditCtrl(T lwr = -1, bool include = true) :
		range_lwr_(lwr),
		range_inc_(include)
	{}

    BEGIN_MSG_MAP(CHalEditCtrl<T>)
		REFLECTED_NOTIFY_CODE_HANDLER_EX(EN_KILLFOCUS, OnKillFocus)
		
        DEFAULT_REFLECTION_HANDLER()
    END_MSG_MAP()

	void Attach(HWND hWndNew)
	{
		ATLASSERT(::IsWindow(hWndNew));
        baseClass::SubclassWindow(hWndNew);
	}
	
	LRESULT OnKillFocus(LPNMHDR lpHdr)
	{
		const int buffer_size = 512;
		boost::array<wchar_t, buffer_size> buffer;
		GetWindowText(buffer.elems, buffer_size);
		
		try
		{
		T val = lexical_cast<T>(buffer.elems);
	
		if (range_inc_)
			if (value_ < range_lwr_) val = -1;
		else
			if (value_ <= range_lwr_) val = -1;
			
		}        
		catch(boost::bad_lexical_cast &)
		{
		value_ = -1;
		}
		
		if (value_ < 0)	SetWindowText(L"∞");
		
		SetMsgHandled(false);
		
		return 0;
	}
	
	T Value() { return value_; }
	
private:
	T value_;
	
	T range_lwr_;
	bool range_inc_;
};

typedef CHalEditCtrl<int> CHalIntegerEditCtrl;
