
#pragma once

#include <boost/archive/xml_oarchive.hpp>
#include <boost/archive/xml_iarchive.hpp>

class WindowConfig
{
public:
	WindowConfig() :
		splitterPos(100),
		use_tray(true)
	{
		rect.top = 10;
		rect.left = 10;
		rect.bottom = 400;
		rect.right = 620;
		
		mainListColWidth[0] = 100;
		mainListColWidth[1] = 110;
		mainListColWidth[2] = 60;
		mainListColWidth[3] = 60;
		mainListColWidth[4] = 60;
		mainListColWidth[5] = 42;
		mainListColWidth[6] = 45;
		
		mainListColWidthEx[0] = 45;
		mainListColWidthEx[1] = 45;	
	}
	
    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive& ar, const unsigned int version)
	{	
		ar & BOOST_SERIALIZATION_NVP(rect);
		ar & BOOST_SERIALIZATION_NVP(splitterPos);
		ar & BOOST_SERIALIZATION_NVP(mainListColWidth);
		ar & BOOST_SERIALIZATION_NVP(use_tray);
		if (version > 0) {
			ar & BOOST_SERIALIZATION_NVP(mainListColWidthEx);
		}
	}
	
	friend class GeneralOptions;
	friend class HaliteWindow;
	friend class HaliteListViewCtrl;
	friend int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow);
	
private:	
	static const size_t numMainCols = 7;
	static const size_t numMainColsEx = 2;
	CRect rect;
	unsigned int splitterPos;
	unsigned int mainListColWidth[numMainCols];
	unsigned int mainListColWidthEx[numMainColsEx];
	bool use_tray;
};

BOOST_CLASS_VERSION(WindowConfig, 1)
