#ifndef CSTDOUTSTREAM_HPP
#define CSTDOUTSTREAM_HPP

#include <ostream>
#include <cstdint>

#include <7zip/IStream.h>
#include <Common/MyCom.h>

namespace bit7z {
    using std::ostream;

    class CStdOutStream : public IOutStream, public CMyUnknownImp {
        public:
            explicit CStdOutStream( std::ostream& outputStream );

            virtual ~CStdOutStream() = default;

            MY_UNKNOWN_IMP1( IOutStream )

            // IOutStream
            STDMETHOD( Write )( void const* data, UInt32 size, UInt32* processedSize );
            STDMETHOD( Seek )( Int64 offset, UInt32 seekOrigin, UInt64* newPosition );
            STDMETHOD( SetSize )( UInt64 newSize );

        private:
            ostream& mOutputStream;
    };
}

#endif // CSTDOUTSTREAM_HPP
