#ifndef MEMUPDATECALLBACK_HPP
#define MEMUPDATECALLBACK_HPP

#include "7zip/Archive/IArchive.h"
#include "7zip/IPassword.h"
#include "Common/MyCom.h"

#include "../include/fsindexer.hpp"
#include "../include/callback.hpp"
#include "../include/bittypes.hpp"
#include "../include/bitarchivecreator.hpp"

namespace bit7z {
    using namespace filesystem;
    using std::vector;
    using std::wstring;

    class MemUpdateCallback : public IArchiveUpdateCallback, ICryptoGetTextPassword2, CMyUnknownImp, public Callback {
        public:
            MY_UNKNOWN_IMP2( IArchiveUpdateCallback2, ICryptoGetTextPassword2 )

            // IProgress
            STDMETHOD( SetTotal )( UInt64 size );
            STDMETHOD( SetCompleted )( const UInt64 * completeValue );

            // IArchiveUpdateCallback
            STDMETHOD( EnumProperties )( IEnumSTATPROPSTG * *enumerator );
            STDMETHOD( GetUpdateItemInfo )( UInt32 index, Int32 * newData, Int32 * newProperties,
                                            UInt32 * indexInArchive );
            STDMETHOD( GetProperty )( UInt32 /*index*/, PROPID propID, PROPVARIANT * value );
            STDMETHOD( GetStream )( UInt32 /*index*/, ISequentialInStream * *inStream );
            STDMETHOD( SetOperationResult )( Int32 operationResult );

            //ICryptoGetTextPassword2
            STDMETHOD( CryptoGetTextPassword2 )( Int32 * passwordIsDefined, BSTR * password );

        public:
            const BitArchiveCreator& mCreator;
            vector< UInt64 > mVolumesSizes;
            wstring mVolName;
            wstring mVolExt;

            wstring mDirPrefix;

            bool mAskPassword;

            bool mNeedBeClosed;

            vector< wstring > mFailedFiles;
            vector< HRESULT > mFailedCodes;

            const vector< byte_t >& mBuffer;
            const wstring& mBufferName;

            MemUpdateCallback( const BitArchiveCreator& creator, const vector< byte_t >& out_buffer, const wstring& buffer_name );
            virtual ~MemUpdateCallback();

            HRESULT Finilize();
    };
}
#endif // MEMUPDATECALLBACK_HPP
