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
    string narrow( const wstring& wideString );
    wstring widen( const string& narrowString );
}

#endif //UTIL_HPP
