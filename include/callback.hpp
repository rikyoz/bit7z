#ifndef CALLBACK_HPP
#define CALLBACK_HPP

#include <string>

using namespace std;

namespace Bit7z {
    class Callback {
        public:
            void setPassword( wstring const& password );
            wstring getErrorMessage();

        protected:
            Callback();

            wstring mPassword;
            wstring mErrorMessage;
    };
}

#endif // CALLBACK_HPP
