#ifndef COUTSTREAM_HPP
#define COUTSTREAM_HPP

#include <ostream>

#include "7zip/IStream.h"
#include "Common/MyCom.h"

namespace bit7z {
    using std::ostream;

    class COutStdStream : public IOutStream, public CMyUnknownImp {
        public:
            explicit COutStdStream( std::ostream& outputStream );

            virtual ~COutStdStream();

            MY_UNKNOWN_IMP1( IOutStream );

            // IOutStream
            STDMETHOD( Write )( void const* data, uint32_t size, uint32_t* processedSize );
            STDMETHOD( Seek )( int64_t offset, uint32_t seekOrigin, uint64_t* newPosition );
            STDMETHOD( SetSize )( uint64_t newSize );

        private:
            ostream& mOutputStream;
    };
}

#endif // COUTSTREAM_HPP
