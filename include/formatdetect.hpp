#ifndef FORMATDETECT_HPP
#define FORMATDETECT_HPP

#ifdef BIT7Z_AUTO_FORMAT

#include "../include/bitformat.hpp"

#include <7zip/IStream.h>

namespace bit7z {
    const BitInFormat& detectFormatFromExt( const tstring& in_file );
    const BitInFormat& detectFormatFromSig( IInStream* stream );
}
#endif

#endif