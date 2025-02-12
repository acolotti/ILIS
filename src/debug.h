#ifndef _DEBUG_H_
#define _DEBUG_H_

#include <iostream>
#include <fstream>

using namespace std;

class debug{
	public:
		debug(bool active);
		void setState(bool active);
		template <typename T>
		debug& operator<<(T&& t){
            if(active){
                cout << t;
            }
            return *this;
        }
		// this is the type of std::cout
        typedef std::basic_ostream<char, std::char_traits<char> > CoutType;
        // this is the function signature of std::endl
        typedef CoutType& (*StandardEndLine)(CoutType&);
        // define an operator<< to take in std::endl
        debug& operator<<(StandardEndLine manip){
            if (active){
	            manip(std::cout);
            }
            return *this;
        }

	private:
		bool active;
};

#endif
