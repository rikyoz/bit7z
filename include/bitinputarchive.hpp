#ifndef BITINPUTARCHIVE_H
#define BITINPUTARCHIVE_H

#include "../include/bitarchivehandler.hpp"
#include "../include/bitformat.hpp"
#include "../include/bitpropvariant.hpp"
#include "../include/bittypes.hpp"

#include <vector>
#include <string>
#include <cstdint>

struct IInStream;
struct IInArchive;
struct IOutArchive;
struct IArchiveExtractCallback;

namespace bit7z {
    using std::wstring;
    using std::vector;

    class BitInputArchive {
        public:
            BitInputArchive( const BitArchiveHandler& handler, const wstring& in_file );

            BitInputArchive( const BitArchiveHandler& handler, const vector< byte_t >& in_buffer );

            virtual ~BitInputArchive();

            /**
             * @return the detected format of the file.
             */
            const BitInFormat& detectedFormat() const;

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
             * @return the number of items contained in the archive.
             */
            uint32_t itemsCount() const;

            bool isItemFolder( uint32_t index ) const;

            bool isItemEncrypted( uint32_t index ) const;

        protected:
            IInArchive* openArchiveStream( const BitArchiveHandler& handler,
                                           const wstring& name,
                                           IInStream* in_stream );

            HRESULT initUpdatableArchive( IOutArchive** newArc ) const;

            HRESULT extract( const vector< uint32_t >& indices, IArchiveExtractCallback* callback ) const;

            HRESULT test( IArchiveExtractCallback* callback ) const;

            HRESULT close() const;

            IInArchive* mInArchive;
            const BitInFormat* mDetectedFormat;

            friend class BitExtractor;
            friend class BitMemExtractor;
            friend class BitCompressor;
    };
}

#endif //BITINPUTARCHIVE_H
