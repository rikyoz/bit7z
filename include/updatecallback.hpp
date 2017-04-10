#ifndef UPDATECALLBACK_HPP
#define UPDATECALLBACK_HPP

#include "7zip/Archive/IArchive.h"
#include "7zip/ICoder.h"
#include "7zip/IPassword.h"
#include "Common/MyCom.h"

#include "../include/fsindexer.hpp"
#include "../include/callback.hpp"
#include "../include/bitarchivecreator.hpp"

namespace bit7z {
    using namespace filesystem;
    using std::vector;
    using std::wstring;

    class UpdateCallback : public IArchiveUpdateCallback2, public ICompressProgressInfo,
            ICryptoGetTextPassword2, CMyUnknownImp, public Callback {
        public:
            vector< wstring > mFailedFiles;

            explicit UpdateCallback( const BitArchiveCreator& creator, const vector< FSItem >& dirItems );
            virtual ~UpdateCallback();

            HRESULT Finilize();

            MY_UNKNOWN_IMP3( IArchiveUpdateCallback2, ICompressProgressInfo, ICryptoGetTextPassword2 )

            // IProgress
            STDMETHOD( SetTotal )( UInt64 size );
            STDMETHOD( SetCompleted )( const UInt64 * completeValue );

            // ICompressProgressInfo
            STDMETHOD( SetRatioInfo )( const UInt64 *inSize, const UInt64 *outSize );

            // IArchiveUpdateCallback2
            STDMETHOD( EnumProperties )( IEnumSTATPROPSTG * *enumerator );
            STDMETHOD( GetUpdateItemInfo )( UInt32 index, Int32 * newData, Int32 * newProperties,
                                            UInt32 * indexInArchive );
            STDMETHOD( GetProperty )( UInt32 index, PROPID propID, PROPVARIANT * value );
            STDMETHOD( GetStream )( UInt32 index, ISequentialInStream * *inStream );
            STDMETHOD( SetOperationResult )( Int32 operationResult );
            STDMETHOD( GetVolumeSize )( UInt32 index, UInt64 * size );
            STDMETHOD( GetVolumeStream )( UInt32 index, ISequentialOutStream * *volumeStream );

            //ICryptoGetTextPassword2
            STDMETHOD( CryptoGetTextPassword2 )( Int32 * passwordIsDefined, BSTR * password );

        private:
            uint64_t mVolSize;
            wstring mVolName;
            wstring mVolExt;

            wstring mDirPrefix;
            const vector< FSItem >& mDirItems;
            const BitArchiveCreator& mCreator;

            bool mAskPassword;

            bool mNeedBeClosed;

            vector< HRESULT > mFailedCodes;
    };
}
#endif // UPDATECALLBACK_HPP
