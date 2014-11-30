#ifndef OPENCALLBACK_HPP
#define OPENCALLBACK_HPP

#include "7zip/Archive/IArchive.h"
#include "7zip/IPassword.h"
#include "Common/MyCom.h"
#include "Windows/COM.h"

namespace Bit7z {
    class OpenCallback: public IArchiveOpenCallback, ICryptoGetTextPassword, CMyUnknownImp {
        public:
            OpenCallback();

            MY_UNKNOWN_IMP1( ICryptoGetTextPassword )

            virtual HRESULT SetTotal( const UInt64* files, const UInt64* bytes );
            virtual HRESULT SetCompleted( const UInt64* files, const UInt64* bytes );

            virtual HRESULT CryptoGetTextPassword( BSTR* password );

            void setPassword( const UString& );

        private:
            bool passwordIsDefined;
            UString password;
    };
}

#endif // OPENCALLBACK_HPP
