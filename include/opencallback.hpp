#ifndef OPENCALLBACK_HPP
#define OPENCALLBACK_HPP

#include <string>

#include "7zip/Archive/IArchive.h"
#include "7zip/IPassword.h"
#include "Common/MyCom.h"

#include "../include/callback.hpp"
#include "../include/fsitem.hpp"
#include "../include/bitarchiveopener.hpp"

namespace bit7z {
    using filesystem::FSItem;

    class OpenCallback : public IArchiveOpenCallback, public IArchiveOpenVolumeCallback,
        ICryptoGetTextPassword, CMyUnknownImp, public Callback {
        public:
            OpenCallback( const BitArchiveOpener& opener, const std::wstring &filename = L"." );
            virtual ~OpenCallback();

            MY_UNKNOWN_IMP3( IArchiveOpenVolumeCallback, IArchiveOpenSetSubArchiveName, ICryptoGetTextPassword )

            //IArchiveOpenCallback
            STDMETHOD( SetTotal )( const UInt64* files, const UInt64* bytes );
            STDMETHOD( SetCompleted )( const UInt64* files, const UInt64* bytes );

            //IArchiveOpenVolumeCallback
            STDMETHOD( GetProperty )( PROPID propID, PROPVARIANT* value );
            STDMETHOD( GetStream )( const wchar_t* name, IInStream** inStream );

            //IArchiveOpenSetSubArchiveName
            STDMETHOD( SetSubArchiveName )( const wchar_t *name );

            //ICryptoGetTextPassword
            STDMETHOD( CryptoGetTextPassword )( BSTR* password );

        private:
            const BitArchiveOpener& mOpener;
            bool mSubArchiveMode;
            wstring mSubArchiveName;
            FSItem mFileItem;
    };
}
#endif // OPENCALLBACK_HPP
