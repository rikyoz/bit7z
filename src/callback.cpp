#include "../include/callback.hpp"

using namespace std;
using namespace bit7z;

Callback::Callback() : mErrorMessage( L"" ) {}

wstring Callback::getErrorMessage() const {
    return mErrorMessage;
}
