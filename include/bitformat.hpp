#ifndef BITFORMAT
#define BITFORMAT

#include <bitset>
#include <string>

#include "../include/bitguids.hpp"

#define FEATURES_COUNT 6

namespace bit7z {
    using std::wstring;
    using std::bitset;

    /**
     * @brief The FormatFeatures enum specifies the features supported by an archive file format.
     */
    enum FormatFeatures {
        MULTIPLE_FILES    = 1 << 0,///< The format can compress/extract multiple files (2^0 = 000001)
        SOLID_ARCHIVE     = 1 << 1,///< The format supports solid archives (2^1 = 000010)
        COMPRESSION_LEVEL = 1 << 2,///< The format is able to use different compression levels (2^2 = 000100)
        ENCRYPTION        = 1 << 3,///< The format supports archive encryption (2^3 = 001000)
        HEADER_ENCRYPTION = 1 << 4,///< The format can encrypt the file names (2^4 = 010000)
        INMEM_COMPRESSION = 1 << 5,///< The format is able to create archives in-memory (2^5 = 100000)
    };

    /**
     * @brief The BitInFormat class specifies an extractable archive format.
     *
     * @note Usually, the user of the library should not create new formats and, instead,
     * use the ones provided by the BitFormat namespace.
     */
    class BitInFormat {
        public:
            /**
             * @brief Constructs a BitInFormat object with the id value used by the 7z SDK.
             * @param value  the value of the format in the 7z SDK.
             */
            explicit BitInFormat( unsigned char value );

            /**
             * @return the value of the format in the 7z SDK.
             */
            int value() const;

            /**
             * @return the GUID that identifies the file format in the 7z SDK.
             */
            const GUID guid() const;

            /**
             * @param other  the target object to compare to.
             * @return true if this format is equal to "other".
             */
            bool operator==( BitInFormat const &other ) const;

            /**
             * @param other  the target object to compare to.
             * @return true if this format is not equal to "other".
             */
            bool operator!=( BitInFormat const &other ) const;

        private:
            const unsigned char mValue;
    };

    /**
     * @brief The BitInOutFormat class specifies a format available for creating new archives and extract old ones
     *
     * @note Usually, the user of the library should not create new formats and, instead,
     * use the ones provided by the BitFormat namespace
     */
    class BitInOutFormat : public BitInFormat {
        public:
            /**
             * @brief Constructs a BitInOutFormat object with a id value, an extension and a set of supported features
             * @param value     the value of the format in the 7z SDK
             * @param ext       the default file extension of the archive format
             * @param features  the set of features supported by the archive format
             */
            BitInOutFormat( unsigned char value, const wstring &ext, bitset< FEATURES_COUNT > features );

            /**
             * @return the default file estension of the archive format
             */
            const wstring& extension() const;

            /**
             * @return the bitset of the features supported by the format
             */
            const bitset< FEATURES_COUNT > features() const;

            /**
             * @brief Checks if the format has a specific feature (see FormatFeatures enum)
             * @param feature   feature to be checked
             * @return true if the format has the feature, false otherwise
             */
            bool hasFeature( FormatFeatures feature ) const;

        private:
            const wstring mExtension;
            const bitset< FEATURES_COUNT > mFeatures;
    };

    /**
     * @brief The namespace BitFormat contains a set of archive formats usable with bit7z classes
     */
    namespace BitFormat {
        extern const BitInFormat Rar,       ///< RAR Archive Format
                                 Arj,       ///< ARJ Archive Format
                                 Z,         ///< Z Archive Format
                                 Lzh,       ///< LZH Archive Format
                                 Cab,       ///< CAB Archive Format
                                 Nsis,      ///< NSIS Archive Format
                                 Lzma,      ///< LZMA Archive Format
                                 Lzma86,    ///< LZMA86 Archive Format
                                 Ppmd,      ///< PPMD Archive Format
                                 Ext,       ///< EXT Archive Format
                                 VMDK,      ///< VMDK Archive Format
                                 VDI,       ///< VDI Archive Format
                                 QCow,      ///< QCOW Archive Format
                                 GPT,       ///< GPT Archive Format
                                 Rar5,      ///< RAR5 Archive Format
                                 IHex,      ///< IHEX Archive Format
                                 Hxs,       ///< HXS Archive Format
                                 TE,        ///< TE Archive Format
                                 UEFIc,     ///< UEFIc Archive Format
                                 UEFIs,     ///< UEFIs Archive Format
                                 SquashFS,  ///< SquashFS Archive Format
                                 CramFS,    ///< CramFS Archive Format
                                 APM,       ///< APM Archive Format
                                 Mslz,      ///< MSLZ Archive Format
                                 Flv,       ///< FLV Archive Format
                                 Swf,       ///< SWF Archive Format
                                 Swfc,      ///< SWFC Archive Format
                                 Ntfs,      ///< NTFS Archive Format
                                 Fat,       ///< FAT Archive Format
                                 Mbr,       ///< MBR Archive Format
                                 Vhd,       ///< VHD Archive Format
                                 Pe,        ///< PE Archive Format
                                 Elf,       ///< ELF Archive Format
                                 Macho,     ///< MACHO Archive Format
                                 Udf,       ///< UDF Archive Format
                                 Xar,       ///< XAR Archive Format
                                 Mub,       ///< MUB Archive Format
                                 Hfs,       ///< HFS Archive Format
                                 Dmg,       ///< DMG Archive Format
                                 Compound,  ///< COMPOUND Archive Format
                                 Iso,       ///< ISO Archive Format
                                 Chm,       ///< CHM Archive Format
                                 Split,     ///< SPLIT Archive Format
                                 Rpm,       ///< RPM Archive Format
                                 Deb,       ///< DEB Archive Format
                                 Cpio;      ///< CPIO Archive Format

        extern const BitInOutFormat Zip,        ///< ZIP Archive Format
                                    BZip2,      ///< BZIP2 Archive Format
                                    SevenZip,   ///< 7Z Archive Format
                                    Xz,         ///< XZ Archive Format
                                    Wim,        ///< WIM Archive Format
                                    Tar,        ///< TAR Archive Format
                                    GZip;       ///< GZIP Archive Format
    }
}
#endif // BITFORMAT
