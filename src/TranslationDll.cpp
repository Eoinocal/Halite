// Copyright Vladimir Prus 2002-2004.
// Distributed under the Boost Software License, Version 1.0.
#include <windows.h>

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <iterator>
#include <fstream>
#include <algorithm>

using std::cout;
using std::cerr;
using std::string;
using std::wstring;

#include <boost/xpressive/xpressive.hpp>
using namespace boost::xpressive;

#include <boost/program_options.hpp>
namespace po = boost::program_options;

#include <boost/foreach.hpp>
#define foreach BOOST_FOREACH

#include "global/string_conv.hpp"

// A helper function to simplify the main part.
template<class T>
std::ostream& operator<<(std::ostream& os, const std::vector<T>& v)
{
    copy(v.begin(), v.end(), std::ostream_iterator<T>(std::cout, " ")); 
    return os;
}

template<typename tstr, typename tstream>
void load_file(tstr& s, tstream& is)
{
   s.erase();
   if(is.bad()) return;
   s.reserve(is.rdbuf()->in_avail());
   typename tstr::value_type c;
   while(is.get(c))
   {
      if(s.capacity() == s.size())
         s.reserve(s.capacity() * 3);
      s.append(1, c);
   }
}

using namespace std;
typedef codecvt<wchar_t, char, mbstate_t> Mybase;

    // CLASS Simple_codecvt

class Simple_codecvt : public Mybase
{
public:
    typedef wchar_t  _E;
    typedef char  _To;
    typedef mbstate_t _St;

    explicit Simple_codecvt(size_t _R = 0)
        : Mybase(_R) {}

protected:
    virtual result do_in(_St& _State,
  const _To *_F1, const _To *_L1, const _To *&_Mid1,
  _E *F2, _E *_L2, _E *&_Mid2) const
 {return (noconv);}

    virtual result do_out(_St& _State,
  const _E *_F1, const _E *_L1, const _E *&_Mid1,
  _To *F2, _E *_L2, _To *&_Mid2) const
 {return (noconv);}

    virtual result do_unshift(_St& _State,
  _To *_F2, _To *_L2, _To *&_Mid2) const
 {return (noconv);}

    virtual int do_length(_St& _State, const _To *_F1,
  const _To *_L1, size_t _N2) const _THROW0()
 {return (_N2 < (size_t)(_L1 - _F1)
 ? _N2 : _L1 - _F1); }

    virtual bool do_always_noconv() const _THROW0()
 {return (true);}

    virtual int do_max_length() const _THROW0()
 {return (2);}

    virtual int do_encoding() const _THROW0()
 {return (2);}

}; 

const char* expression_text = "(^[[:blank:]]*#(?:[^\\\\\\n]|\\\\[^\\n[:punct:][:word:]]*[\\n[:punct:][:word:]])*)|"
								 "(//[^\\n]*|/\\*.*?\\*/)";
const char* format_string = "(?1<font color=\"#008040\">$&</font>)"
							   "(?2<I><font color=\"#000080\">$&</font></I>)";

int main(int ac, char* av[])
{
    try 
	{
		int opt;
		po::options_description desc("Allowed options");
		desc.add_options()
			("help", "produce help message")
			("optimization", po::value<int>(&opt)->default_value(10), "optimization level")
			("include-path,I", po::value<std::vector<string> >(), "include path")
			("input-file", po::value<std::vector<string> >(), "input file")
		;

		po::positional_options_description p;
		p.add("input-file", -1);
		
		po::variables_map vm;
		po::store(po::command_line_parser(ac, av).options(desc).positional(p).run(), vm);
		po::notify(vm);  

        if (vm.count("help")) {
            cout << desc << "\n";
            return 1;
        }

		if (vm.count("include-path"))
		{
			cout << "Include paths are: " 
				 << vm["include-path"].as<std::vector<string> >() << ".\n";
		}
		
		if (vm.count("input-file"))
		{
			cout << "Input files are: " 
				 << vm["input-file"].as<std::vector<string> >() << ".\n";
		}
		
		cout << "Optimization level is " << opt << ".\n";
		
		std::wifstream fs(L"Halite.rc", std::ios::binary);
		wstring in_file;
		load_file(in_file, fs);
		fs.close();	
				
		cout << "Loaded File at size " << in_file.size() << "\n";
		
	//	wstring win = hal::from_utf8(string(in_file.begin(), in_file.end()));
		
	//	cout << "Converted file at size " << win.size() << "\n";
		
		wstring in = L"A ЏиϊсoδΣ Hello //World";
		wsregex comments = wsregex::compile(L"(//[^\\n]*|/\\*.*?\\*/)");
		
		wstring clear(L"");	
		wstring out = regex_replace(in, comments, clear);
		
		cout << "Converted file at size " << out.size() << "\n";

		MessageBox(0, out.c_str(), L"Hi", 0);
		std::wofstream os;
		
    locale loc = _ADDFAC(locale::classic(), new Simple_codecvt); 
     os.imbue(loc); 
	os.open(L"Hal.rc", std::ios::binary);
	/*	foreach (wchar_t c, out)
		{
			os.put(c);
		}*/
		//os.write(out.c_str(), out.size());
		os << out;
		os.close();		
    }
    catch(std::exception& e) 
	{
        cerr << "error: " << e.what() << "\n";
        return 1;
    }
    catch(...) 
	{
        cerr << "Exception of unknown type!\n";
    }

    return 0;
}




