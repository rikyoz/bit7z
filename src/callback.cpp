// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#include "../include/callback.hpp"

using namespace std;
using namespace bit7z;

Callback::Callback() : mErrorMessage( L"" ) {}

wstring Callback::getErrorMessage() const {
    return mErrorMessage;
}
