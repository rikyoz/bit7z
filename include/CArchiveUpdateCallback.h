#ifndef CARCHIVEUPDATECALLBACK_H
#define CARCHIVEUPDATECALLBACK_H

#include "7zip/Archive/IArchive.h"
#include "7zip/Common/FileStreams.h"
#include "7zip/IPassword.h"

#include "Common/MyCom.h"

#include "Windows/COM.h"

#include "util.h"

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

class CArchiveUpdateCallback:
    public IArchiveUpdateCallback2,
    public ICryptoGetTextPassword2,
    public CMyUnknownImp {
    public:
        MY_UNKNOWN_IMP2( IArchiveUpdateCallback2, ICryptoGetTextPassword2 )

        // IProgress
        STDMETHOD( SetTotal )( UInt64 size );
        STDMETHOD( SetCompleted )( const UInt64* completeValue );

        // IUpdateCallback2
        STDMETHOD( EnumProperties )( IEnumSTATPROPSTG** enumerator );
        STDMETHOD( GetUpdateItemInfo )( UInt32 index,
                                        Int32* newData, Int32* newProperties, UInt32* indexInArchive );
        STDMETHOD( GetProperty )( UInt32 index, PROPID propID, PROPVARIANT* value );
        STDMETHOD( GetStream )( UInt32 index, ISequentialInStream** inStream );
        STDMETHOD( SetOperationResult )( Int32 operationResult );
        STDMETHOD( GetVolumeSize )( UInt32 index, UInt64* size );
        STDMETHOD( GetVolumeStream )( UInt32 index, ISequentialOutStream** volumeStream );

        STDMETHOD( CryptoGetTextPassword2 )( Int32* passwordIsDefined, BSTR* password );

    public:
        CRecordVector<UInt64> VolumesSizes;
        UString VolName;
        UString VolExt;

        UString DirPrefix;
        const CObjectVector<CDirItem>* DirItems;

        bool PasswordIsDefined;
        UString Password;
        bool AskPassword;

        bool m_NeedBeClosed;

        UStringVector FailedFiles;
        CRecordVector<HRESULT> FailedCodes;

        CArchiveUpdateCallback(): PasswordIsDefined( false ), AskPassword( false ), DirItems( 0 ) {}

        ~CArchiveUpdateCallback() { Finilize(); }
        HRESULT Finilize();

        void Init( const CObjectVector<CDirItem>* dirItems ) {
            DirItems = dirItems;
            m_NeedBeClosed = false;
            FailedFiles.Clear();
            FailedCodes.Clear();
        }
};

#endif // CARCHIVEUPDATECALLBACK_H
