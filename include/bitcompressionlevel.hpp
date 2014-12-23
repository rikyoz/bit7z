#ifndef BITCOMPRESSIONLEVEL
#define BITCOMPRESSIONLEVEL

namespace bit7z {

    /**
     * @enum BitCompressionLevel
     * @brief The BitCompressionLevel enum represents the compression level used by 7z when creating archives
     * Note that it uses the same values as in the 7z SDK
     */
    enum BitCompressionLevel { None = 0, Fastest = 1, Fast = 3, Normal = 5, Max = 7, Ultra = 9 };

}

#endif // BITCOMPRESSIONLEVEL

