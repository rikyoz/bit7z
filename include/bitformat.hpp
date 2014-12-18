#ifndef BITFORMAT
#define BITFORMAT

#include <string>

#include "../include/bitguids.hpp"

using namespace std;

namespace bit7z {

    class BitOutFormat {
        public:
            enum { Zip, BZip2, SevenZip, Xz, Wim, Tar, GZip };

            BitOutFormat( int value );
            bool operator==( BitOutFormat const& other ) const;
            bool operator!=( BitOutFormat const& other ) const;
            bool operator==( int const& other ) const;
            bool operator!=( int const& other ) const;
            operator unsigned int() const;
            int value() const;
            virtual const GUID guid() const;

        protected:
            int mValue;
    };

    class BitInFormat : public BitOutFormat {
        public:
            enum { Rar = BitOutFormat::GZip + 1, Arj, Z, Lzh, Cab, Nsis, Lzma, Lzma86, Ppmd,
                   TE, UEFIc, UEFIs, SquashFS, CramFS, APM, Mslz, Flv, Swf, Swfc, Ntfs, Fat,
                   Mbr, Vhd, Pe, Elf, Macho, Udf, Xar, Mub, Hfs, Dmg, Compound, Iso,
                   Chm, Split, Rpm, Deb, Cpio
                 };

            BitInFormat( int value );
            const GUID guid() const;
    };

}

#endif // BITFORMAT
