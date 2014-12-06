#ifndef EXTRACTCALLBACK_HPP
#define EXTRACTCALLBACK_HPP

#include <string>

#include "7zip/Archive/IArchive.h"
#include "7zip/Common/FileStreams.h"
#include "7zip/IPassword.h"
#include "Common/MyCom.h"
#include "Windows/COM.h"

#include "../include/bitguids.hpp"

using namespace std;

namespace Bit7z {
    class ExtractCallback: public IArchiveExtractCallback, ICryptoGetTextPassword, CMyUnknownImp {
        public:
            ExtractCallback(IInArchive* archiveHandler, const wstring& directoryPath );
            void setPassword( const wstring& password );

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
            CMyComPtr<IInArchive> mArchiveHandler;
            wstring mDirectoryPath;  // Output directory
            wstring mFilePath;       // name inside arcvhive
            wstring mDiskFilePath;   // full path to file on disk
            bool mExtractMode;
            struct CProcessedFileInfo {
                FILETIME MTime;
                UInt32 Attrib;
                bool isDir;
                bool AttribDefined;
                bool MTimeDefined;
            } _processedFileInfo;

            COutFileStream* mOutFileStreamSpec;
            CMyComPtr<ISequentialOutStream> mOutFileStream;

            UInt64 mNumErrors;
            //bool mHasPassword;
            wstring mPassword;
    };
}

#endif // EXTRACTCALLBACK_HPP
