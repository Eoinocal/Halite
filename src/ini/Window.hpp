
#pragma once

#include <boost/archive/xml_oarchive.hpp>
#include <boost/archive/xml_iarchive.hpp>

class WindowConfig
{
public:
	WindowConfig() :
		splitterPos(300)
	{
		rect.top = 10;
		rect.left = 10;
		rect.bottom = 400;
		rect.right = 500;
		
		for(size_t i=0; i<numMainCols; ++i)
			mainListColWidth[i] = 50;
	}
	
    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive& ar, const unsigned int version)
	{	
		ar & BOOST_SERIALIZATION_NVP(rect);
		ar & BOOST_SERIALIZATION_NVP(splitterPos);
		ar & BOOST_SERIALIZATION_NVP(mainListColWidth);
	}
	
	friend class HaliteWindow;
	friend int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow);
	
private:	
	static const size_t numMainCols = 7;
	CRect rect;
	unsigned int splitterPos;
	unsigned int mainListColWidth[numMainCols];
};
