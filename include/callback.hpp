#ifndef CALLBACK_HPP
#define CALLBACK_HPP

#include <string>

namespace bit7z {

    using std::wstring;

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
