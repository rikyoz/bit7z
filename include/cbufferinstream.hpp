#ifndef CBUFFERINSTREAM_HPP
#define CBUFFERINSTREAM_HPP

#include <vector>

#include "../include/bittypes.hpp"

#include <7zip/IStream.h>
#include <Common/MyCom.h>

namespace bit7z {
    using std::vector;

    class CBufferInStream : public IInStream, public CMyUnknownImp {
        public:
            explicit CBufferInStream( const vector< byte_t >& in_buffer );

            MY_UNKNOWN_IMP1( IInStream )

            STDMETHOD( Read )( void* data, UInt32 size, UInt32* processedSize );

            STDMETHOD( Seek )( Int64 offset, UInt32 seekOrigin, UInt64* newPosition );

        private:
            const vector< byte_t >& mBuffer;
            size_t mCurrentPosition;
    };
}

#endif // CBUFFERINSTREAM_HPP
