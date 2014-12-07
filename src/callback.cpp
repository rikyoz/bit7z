#include "../include/callback.hpp"

using namespace Bit7z;

Callback::Callback() : mErrorMessage(L"") {}

wstring Callback::getErrorMessage() { return mErrorMessage; }
