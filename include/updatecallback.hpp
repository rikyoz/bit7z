#ifndef UPDATECALLBACK_HPP
#define UPDATECALLBACK_HPP

#include "7zip/Archive/IArchive.h"
#include "7zip/IPassword.h"
#include "Common/MyCom.h"

#include "../include/fsindexer.hpp"
#include "../include/callback.hpp"

using namespace bit7z::filesystem;

namespace bit7z {

    class UpdateCallback : public IArchiveUpdateCallback2, ICryptoGetTextPassword2, CMyUnknownImp, public Callback {
        public:
            MY_UNKNOWN_IMP2( IArchiveUpdateCallback2, ICryptoGetTextPassword2 )

            // IProgress
            STDMETHOD( SetTotal )( UInt64 size );
            STDMETHOD( SetCompleted )( const UInt64* completeValue );

            // IArchiveUpdateCallback2
            STDMETHOD( EnumProperties )( IEnumSTATPROPSTG** enumerator );
            STDMETHOD( GetUpdateItemInfo )( UInt32 index, Int32* newData, Int32* newProperties, UInt32* indexInArchive );
            STDMETHOD( GetProperty )( UInt32 index, PROPID propID, PROPVARIANT* value );
            STDMETHOD( GetStream )( UInt32 index, ISequentialInStream** inStream );
            STDMETHOD( SetOperationResult )( Int32 operationResult );
            STDMETHOD( GetVolumeSize )( UInt32 index, UInt64* size );
            STDMETHOD( GetVolumeStream )( UInt32 index, ISequentialOutStream** volumeStream );

            //ICryptoGetTextPassword2
            STDMETHOD( CryptoGetTextPassword2 )( Int32* passwordIsDefined, BSTR* password );

        public:
            vector<UInt64> mVolumesSizes;
            wstring mVolName;
            wstring mVolExt;

            wstring mDirPrefix;
            const vector<FSItem>& mDirItems;

            bool mAskPassword;

            bool mNeedBeClosed;

            vector<wstring> mFailedFiles;
            vector<HRESULT> mFailedCodes;

            UpdateCallback( const vector<FSItem>& dirItems );
            virtual ~UpdateCallback();

            HRESULT Finilize();
    };

}

#endif // UPDATECALLBACK_HPP
