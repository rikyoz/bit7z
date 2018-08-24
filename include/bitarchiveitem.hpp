#ifndef BITARCHIVEITEMREADER_HPP
#define BITARCHIVEITEMREADER_HPP

#include <cstdint>
#include <string>
#include <map>

#include "../include/bitpropvariant.hpp"

namespace bit7z {
    using std::wstring;
    using std::map;

    class BitArchiveItem {
        public:
            virtual ~BitArchiveItem();

            uint32_t index() const;
            bool isDir() const;
            wstring name() const;
            wstring extension() const;
            wstring path() const;
            uint64_t size() const;
            uint64_t packSize() const;

            BitPropVariant getProperty( BitProperty property ) const;
            map<BitProperty, BitPropVariant> itemProperties() const;

        private:
            const uint32_t mItemIndex;
            map<BitProperty, BitPropVariant> mItemProperties;

            /* BitArchiveItem objects can be created and updated only by BitArchiveReader */
            explicit BitArchiveItem( uint32_t item_index );
            void setProperty( BitProperty property, const BitPropVariant& value );

            friend class BitArchiveInfo;
    };
}

#endif // BITARCHIVEITEMREADER_HPP
