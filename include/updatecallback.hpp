#ifndef UPDATECALLBACK_HPP
#define UPDATECALLBACK_HPP

#include "7zip/Common/FileStreams.h"
#include "7zip/Archive/IArchive.h"
#include "7zip/IPassword.h"
#include "Common/MyCom.h"
#include "Windows/COM.h"

struct CDirItem {
    UInt64 Size;
    FILETIME CTime;
    FILETIME ATime;
    FILETIME MTime;
    UString Name;
    UString FullPath;
    UInt32 Attrib;

    bool isDir() const { return ( Attrib & FILE_ATTRIBUTE_DIRECTORY ) != 0 ; }
};

namespace Bit7z {
    class UpdateCallback : public IArchiveUpdateCallback2, ICryptoGetTextPassword2, CMyUnknownImp {
        public:
            MY_UNKNOWN_IMP2( IArchiveUpdateCallback2, ICryptoGetTextPassword2 )

            // IProgress
            virtual HRESULT SetTotal( UInt64 size );
            virtual HRESULT SetCompleted( const UInt64* completeValue );

            // IUpdateCallback2
            virtual HRESULT EnumProperties( IEnumSTATPROPSTG** enumerator );
            virtual HRESULT GetUpdateItemInfo( UInt32 index, Int32* newData, Int32* newProperties,
                                               UInt32* indexInArchive );
            virtual HRESULT GetProperty( UInt32 index, PROPID propID, PROPVARIANT* value );
            virtual HRESULT GetStream( UInt32 index, ISequentialInStream** inStream );
            virtual HRESULT SetOperationResult( Int32 operationResult );
            virtual HRESULT GetVolumeSize( UInt32 index, UInt64* size );
            virtual HRESULT GetVolumeStream( UInt32 index, ISequentialOutStream** volumeStream );

            //ICryptoGetTextPassword2
            virtual HRESULT CryptoGetTextPassword2( Int32* passwordIsDefined, BSTR* password );

        public:
            CRecordVector<UInt64> mVolumesSizes;
            UString mVolName;
            UString mVolExt;

            UString mDirPrefix;
            const CObjectVector<CDirItem>* mDirItems;

            bool mIsPasswordDefined;
            UString mPassword;
            bool mAskPassword;

            bool mNeedBeClosed;

            UStringVector mFailedFiles;
            CRecordVector<HRESULT> mFailedCodes;

            UpdateCallback(): mIsPasswordDefined( false ), mAskPassword( false ), mDirItems( 0 ) {}

            ~UpdateCallback() { Finilize(); }
            HRESULT Finilize();

            void Init( const CObjectVector<CDirItem>* dirItems ) {
                mDirItems = dirItems;
                mNeedBeClosed = false;
                mFailedFiles.Clear();
                mFailedCodes.Clear();
            }
    };
}

#endif // UPDATECALLBACK_HPP
