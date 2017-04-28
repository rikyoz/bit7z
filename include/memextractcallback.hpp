#ifndef MEMEXTRACTCALLBACK_HPP
#define MEMEXTRACTCALLBACK_HPP

#include <string>
#include <vector>

#include "7zip/Archive/IArchive.h"
#include "7zip/Common/FileStreams.h"
#include "7zip/Common/StreamObjects.h"
#include "7zip/IPassword.h"
#include "Common/MyCom.h"

#include "../include/coutmemstream.hpp"
#include "../include/bitguids.hpp"
#include "../include/bitformat.hpp"
#include "../include/bittypes.hpp"
#include "../include/callback.hpp"
#include "../include/bitarchiveopener.hpp"

namespace bit7z {
    using std::vector;

    class MemExtractCallback : public IArchiveExtractCallback, ICryptoGetTextPassword, CMyUnknownImp, public Callback {
        public:
            MemExtractCallback( const BitArchiveOpener& opener, IInArchive* archiveHandler, vector< byte_t >& buffer );
            virtual ~MemExtractCallback();

            MY_UNKNOWN_IMP1( ICryptoGetTextPassword )

            // IProgress
            STDMETHOD( SetTotal )( UInt64 size );
            STDMETHOD( SetCompleted )( const UInt64 * completeValue );

            // IArchiveExtractCallback
            STDMETHOD( GetStream )( UInt32 index, ISequentialOutStream * *outStream, Int32 askExtractMode );
            STDMETHOD( PrepareOperation )( Int32 askExtractMode );
            STDMETHOD( SetOperationResult )( Int32 resultEOperationResult );

            // ICryptoGetTextPassword
            STDMETHOD( CryptoGetTextPassword )( BSTR * aPassword );

        private:
            const BitArchiveOpener& mOpener;
            CMyComPtr< IInArchive > mArchiveHandler;
            vector< byte_t >& mBuffer;
            bool mExtractMode;
            struct CProcessedFileInfo {
                FILETIME MTime;
                UInt32 Attrib;
                bool isDir;
                bool AttribDefined;
                bool MTimeDefined;
            } mProcessedFileInfo;

            COutMemStream* mOutMemStreamSpec;
            CMyComPtr< ISequentialOutStream > mOutMemStream;

            UInt64 mNumErrors;
    };
}
#endif // MEMEXTRACTCALLBACK_HPP
