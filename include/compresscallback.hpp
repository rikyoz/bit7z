#ifndef COMPRESSCALLBACK_HPP
#define COMPRESSCALLBACK_HPP

#include "../include/callback.hpp"

#include "7zip/Archive/IArchive.h"

namespace bit7z {
    struct CompressCallback : public Callback, public IArchiveUpdateCallback2 {
        virtual uint32_t itemsCount() const = 0;
    };
}

#endif // COMPRESSCALLBACK_HPP
