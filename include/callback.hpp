#ifndef CALLBACK_HPP
#define CALLBACK_HPP

#include <string>

using namespace std;

namespace Bit7z {
    class Callback {
        public:
            wstring getErrorMessage();

        protected:
            Callback();

            wstring mErrorMessage;
    };
}

#endif // CALLBACK_HPP
