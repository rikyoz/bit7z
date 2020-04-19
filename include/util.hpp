//
// Created by rik20 on 12/04/2020.
//

#ifndef UTIL_HPP
#define UTIL_HPP

#include <string>

namespace bit7z {
    using std::string;
    using std::wstring;

    string narrow( const wchar_t* wideString, size_t size );
    wstring widen( const string& narrowString );
}

#ifdef _WIN32
#define WIDEN(tstr) tstr
#else
#define WIDEN(tstr) bit7z::widen(tstr)
#endif

#endif //UTIL_HPP
