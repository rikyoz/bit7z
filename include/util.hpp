#ifndef UTIL_HPP
#define UTIL_HPP

#include "7zip/Archive/IArchive.h"
#include "7zip/Common/FileStreams.h"

#include "../include/bit7zlibrary.hpp"
#include "../include/bitcompressionlevel.hpp"

namespace bit7z {
    namespace util {
        CMyComPtr< IOutArchive > initOutArchive( const Bit7zLibrary &lib, const BitInOutFormat &format,
                                                 const BitCompressionLevel compressionLevel,
                                                 const bool cryptHeaders, const bool solidMode );
    }
}

#endif // UTIL_HPP
