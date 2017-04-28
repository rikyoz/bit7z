#ifndef CALLBACK_HPP
#define CALLBACK_HPP

#include <string>

namespace bit7z {
    using std::wstring;

    class Callback {
        public:
            wstring getErrorMessage() const;

        protected:
            Callback();

            wstring mErrorMessage;
    };
}
#endif // CALLBACK_HPP
