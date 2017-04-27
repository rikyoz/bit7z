#ifndef BITCOMPRESSIONLEVEL
#define BITCOMPRESSIONLEVEL

namespace bit7z {
    /**
     * @brief The BitCompressionLevel enum represents the compression level used by 7z when creating archives.
     * @note It uses the same values as in the 7z SDK (https://sevenzip.osdn.jp/chm/cmdline/switches/method.htm#ZipX).
     */
    enum BitCompressionLevel { NONE = 0,    ///< Copy mode (no compression)
                               FASTEST = 1, ///< Fastest compressing
                               FAST = 3,    ///< Fast compressing
                               NORMAL = 5,  ///< Normal compressing
                               MAX = 7,     ///< Maximum compressing
                               ULTRA = 9    ///< Ultra compressing
                             };
}
#endif // BITCOMPRESSIONLEVEL
