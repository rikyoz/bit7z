#ifndef CARCHIVEOPENCALLBACK_H
#define CARCHIVEOPENCALLBACK_H

#include "7zip/Archive/IArchive.h"
#include "7zip/IPassword.h"

#include "Common/MyCom.h"

#include "Windows/COM.h"

#include "util.h"

class CArchiveOpenCallback:
    public IArchiveOpenCallback,
    public ICryptoGetTextPassword,
    public CMyUnknownImp {
    public:
        MY_UNKNOWN_IMP1( ICryptoGetTextPassword )

        STDMETHOD( SetTotal )( const UInt64* files, const UInt64* bytes );
        STDMETHOD( SetCompleted )( const UInt64* files, const UInt64* bytes );

        STDMETHOD( CryptoGetTextPassword )( BSTR* password );

        bool PasswordIsDefined;
        UString Password;

        CArchiveOpenCallback() : PasswordIsDefined( false ) {}
};

#endif // CARCHIVEOPENCALLBACK_H
