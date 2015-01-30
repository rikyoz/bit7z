#ifndef CALLBACK_HPP
#define CALLBACK_HPP

#include <string>

namespace bit7z {

    class Callback {
        public:
            void setPassword( std::wstring const& password );
            bool isPasswordDefined() const;
            std::wstring getErrorMessage() const;

        protected:
            Callback();

            std::wstring mPassword;
            std::wstring mErrorMessage;
    };

}

#endif // CALLBACK_HPP
