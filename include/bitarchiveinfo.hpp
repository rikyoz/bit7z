#ifndef BITARCHIVEINFO_HPP
#define BITARCHIVEINFO_HPP

#include <vector>

#include "../include/bit7zlibrary.hpp"
#include "../include/bitarchiveopener.hpp"
#include "../include/bitarchiveitem.hpp"

struct IInArchive;

namespace bit7z {
    using std::vector;

    /**
     * @brief The BitArchiveInfo class allows to retrieve metadata information of archives and their content.
     */
    class BitArchiveInfo : public BitArchiveOpener {
        public:
            /**
             * @brief Constructs a BitArchiveInfo object, opening the input file.
             *
             * @param lib       the 7z library used.
             * @param in_file   the input archive file path.
             * @param format    the input archive format.
             */
            BitArchiveInfo( const Bit7zLibrary& lib, const wstring& in_file, const BitInFormat& format );

            /**
             * @brief BitArchiveInfo destructor.
             *
             * @note It releases the input archive file.
             */
            virtual ~BitArchiveInfo() override;

            /**
             * @brief Gets the specified archive property.
             *
             * @param property  the property to be retrieved.
             *
             * @return the current value of the archive property or an empty BitPropVariant if no value is specified.
             */
            BitPropVariant getArchiveProperty( BitProperty property ) const;

            /**
             * @brief Gets the specified property of an item in the archive.
             *
             * @param index     the index (in the archive) of the item.
             * @param property  the property to be retrieved.
             *
             * @return the current value of the item property or an empty BitPropVariant if no value is specified.
             */
            BitPropVariant getItemProperty( uint32_t index, BitProperty property ) const;

            /**
             * @return a map of all the available (i.e. non empty) archive properties and their respective values.
             */
            map<BitProperty, BitPropVariant> archiveProperties() const;

            /**
             * @return a vector of all the archive items as BitArchiveItem objects.
             */
            vector< BitArchiveItem > items() const;

            /**
             * @return the number of items contained in the archive.
             */
            uint32_t itemsCount() const;

            /**
             * @return the number of folders contained in the archive.
             */
            uint32_t foldersCount() const;

            /**
             * @return the number of files contained in the archive.
             */
            uint32_t filesCount() const;

            /**
             * @return the total uncompressed size of the archive content.
             */
            uint64_t size() const;

            /**
             * @return the total compressed size of the archive content.
             */
            uint64_t packSize() const;

        private:
            IInArchive* mInArchive;
    };
}

#endif // BITARCHIVEINFO_HPP
