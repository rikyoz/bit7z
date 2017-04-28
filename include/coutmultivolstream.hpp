#ifndef COUTMULTIVOLSTREAM_HPP
#define COUTMULTIVOLSTREAM_HPP

#include <vector>
#include <cstdint>

//#include "7zip/UI/Common/TempFiles.h"
#include "7zip/Common/FileStreams.h"
#include "7zip/IStream.h"
#include "Common/MyCom.h"

using std::vector;
using std::wstring;

class COutMultiVolStream : public IOutStream, public CMyUnknownImp {
        //CTempFiles* TempFiles;
        uint64_t mVolSize;
        wstring  mVolPrefix;
        unsigned mStreamIndex; // required stream
        uint64_t mOffsetPos;   // offset from start of _streamIndex index
        uint64_t mAbsPos;
        uint64_t mLength;

        struct CAltStreamInfo {
            COutFileStream* streamSpec;
            CMyComPtr<IOutStream> stream;
            wstring name;
            uint64_t pos;
            uint64_t realSize;
        };
        vector< CAltStreamInfo > mVolStreams;

    public:
        COutMultiVolStream( uint64_t size, const wstring &archiveName );
        virtual ~COutMultiVolStream();

        bool SetMTime( const FILETIME* mTime );
        HRESULT Close();

        UInt64 GetSize() const;

        MY_UNKNOWN_IMP1( IOutStream )

        STDMETHOD( Write )( const void* data, UInt32 size, UInt32* processedSize );
        STDMETHOD( Seek )( Int64 offset, UInt32 seekOrigin, UInt64* newPosition );
        STDMETHOD( SetSize )( UInt64 newSize );
};

#endif // COUTMULTIVOLSTREAM_HPP
