#ifndef CARCHIVEEXTRACTCALLBACK_H
#define CARCHIVEEXTRACTCALLBACK_H

#include "7zip/Archive/IArchive.h"
#include "7zip/Common/FileStreams.h"
#include "7zip/IPassword.h"

#include "Common/MyCom.h"

#include "Windows/COM.h"

#include "util.h"

class CArchiveExtractCallback:
    public IArchiveExtractCallback,
    public ICryptoGetTextPassword,
    public CMyUnknownImp {
    public:
        MY_UNKNOWN_IMP1( ICryptoGetTextPassword )

        // IProgress
        STDMETHOD( SetTotal )( UInt64 size );
        STDMETHOD( SetCompleted )( const UInt64* completeValue );

        // IArchiveExtractCallback
        STDMETHOD( GetStream )( UInt32 index, ISequentialOutStream** outStream, Int32 askExtractMode );
        STDMETHOD( PrepareOperation )( Int32 askExtractMode );
        STDMETHOD( SetOperationResult )( Int32 resultEOperationResult );

        // ICryptoGetTextPassword
        STDMETHOD( CryptoGetTextPassword )( BSTR* aPassword );

    private:
        CMyComPtr<IInArchive> _archiveHandler;
        UString _directoryPath;  // Output directory
        UString _filePath;       // name inside arcvhive
        UString _diskFilePath;   // full path to file on disk
        bool _extractMode;
        struct CProcessedFileInfo {
            FILETIME MTime;
            UInt32 Attrib;
            bool isDir;
            bool AttribDefined;
            bool MTimeDefined;
        } _processedFileInfo;

        COutFileStream* _outFileStreamSpec;
        CMyComPtr<ISequentialOutStream> _outFileStream;

    public:
        void Init( IInArchive* archiveHandler, const UString& directoryPath );

        UInt64 NumErrors;
        bool PasswordIsDefined;
        UString Password;

        CArchiveExtractCallback() : PasswordIsDefined( false ) {}
};

#endif // CARCHIVEEXTRACTCALLBACK_H
