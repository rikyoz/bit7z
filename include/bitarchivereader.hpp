#ifndef BITARCHIVEINFO_HPP
#define BITARCHIVEINFO_HPP

#include <vector>

#include "../include/bit7zlibrary.hpp"
#include "../include/bitarchiveopener.hpp"
#include "../include/bitarchiveitem.hpp"

struct IInArchive;

namespace bit7z {
    using std::vector;

    class BitArchiveReader : public BitArchiveOpener {
        public:
            BitArchiveReader( const Bit7zLibrary& lib, const wstring& in_file, const BitInFormat& format );
            virtual ~BitArchiveReader() override;

            BitPropVariant getArchiveProperty( BitProperty property ) const;
            BitPropVariant getItemProperty( uint32_t index, BitProperty property ) const;

            map<BitProperty, BitPropVariant> archiveProperties() const;
            vector< BitArchiveItem > items() const;

            uint32_t itemsCount() const;
            uint32_t foldersCount() const;
            uint32_t filesCount() const;
            uint64_t size() const;
            uint64_t packSize() const;

        private:
            IInArchive* mInArchive;
    };
}

#endif // BITARCHIVEINFO_HPP
