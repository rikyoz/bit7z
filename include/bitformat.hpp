#ifndef BITFORMAT
#define BITFORMAT

#include "../include/bitguids.hpp"

namespace bit7z {

    /**
     * @brief The BitOutFormat class specifies the formats available for creating new archives
     */
    class BitOutFormat {
        public:
            enum Format { Zip = 0, BZip2, SevenZip, Xz, Wim, Tar, GZip };

            BitOutFormat( unsigned int value );
            bool operator==( Format const& other ) const;
            bool operator!=( Format const& other ) const;
            operator unsigned int() const;
            unsigned int value() const;
            virtual const GUID* guid() const;

        protected:
            unsigned int mValue;
    };

    /**
     * @brief The BitInFormat class specifies the extractable archive formats
     *
     * @note The set of formats in BitInFormat is a superset of the set of formats in BitOutFormat
     */
    class BitInFormat : public BitOutFormat {
        public:
            enum Format { Rar = BitOutFormat::GZip + 1, Arj, Z, Lzh, Cab, Nsis, Lzma, Lzma86, Ppmd, TE, UEFIc, UEFIs,
                          SquashFS, CramFS, APM, Mslz, Flv, Swf, Swfc, Ntfs, Fat, Mbr, Vhd, Pe, Elf, Macho, Udf, Xar,
                          Mub, Hfs, Dmg, Compound, Iso, Chm, Split, Rpm, Deb, Cpio
                        };

            BitInFormat( unsigned int value );
            const GUID* guid() const;
    };

}

#endif // BITFORMAT
