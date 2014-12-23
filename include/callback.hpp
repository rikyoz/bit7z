#ifndef CALLBACK_HPP
#define CALLBACK_HPP

#include <string>

using namespace std;

namespace bit7z {

    class Callback {
        public:
            void setPassword( wstring const& password );
            bool isPasswordDefined() const;
            wstring getErrorMessage() const;

        protected:
            Callback();

            wstring mPassword;
            wstring mErrorMessage;
    };

}

#endif // CALLBACK_HPP
