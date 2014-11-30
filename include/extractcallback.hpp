#ifndef EXTRACTCALLBACK_HPP
#define EXTRACTCALLBACK_HPP

#include <iostream>

#include "7zip/Archive/IArchive.h"
#include "7zip/Common/FileStreams.h"
#include "7zip/IPassword.h"
#include "Common/MyCom.h"
#include "Windows/COM.h"

#include "../include/bitguids.hpp"

namespace Bit7z {
    class ExtractCallback: public IArchiveExtractCallback, ICryptoGetTextPassword, CMyUnknownImp {
        public:
            ExtractCallback( IInArchive* archiveHandler, const UString& directoryPath );
            void setPassword( const UString& password );

            MY_UNKNOWN_IMP1( ICryptoGetTextPassword )

            //NOTE: STDMETHOD macro is not used (as in original CArchiveExtractCallback) in order
            //      to avoid warnings about throwing exceptions from __stdcall methods!

            // IProgress
            virtual HRESULT SetTotal( UInt64 size );
            virtual HRESULT SetCompleted( const UInt64* completeValue );

            // IArchiveExtractCallback
            virtual HRESULT GetStream( UInt32 index, ISequentialOutStream** outStream, Int32 askExtractMode );
            virtual HRESULT PrepareOperation( Int32 askExtractMode );
            virtual HRESULT SetOperationResult( Int32 resultEOperationResult );

            // ICryptoGetTextPassword
            virtual HRESULT CryptoGetTextPassword( BSTR* aPassword );

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

            UInt64 numErrors;
            bool hasPassword;
            UString password;
    };
}

#endif // EXTRACTCALLBACK_HPP
