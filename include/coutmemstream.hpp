#ifndef CBUFOUTSTREAM_HPP
#define CBUFOUTSTREAM_HPP

#include <vector>

#include "../include/bittypes.hpp"

#include "7zip/IStream.h"
#include "Common/MyCom.h"

namespace bit7z {
    using std::vector;

    class COutMemStream : public ISequentialOutStream, public CMyUnknownImp {
        public:
            explicit COutMemStream( vector< byte_t >& out_buffer );
            virtual ~COutMemStream();

            vector< byte_t >& mBuffer;

            MY_UNKNOWN_IMP

            STDMETHOD( Write )( const void* data, UInt32 size, UInt32 * processedSize );
    };
}
#endif // CBUFOUTSTREAM_HPP
