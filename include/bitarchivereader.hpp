/*
 * bit7z - A C++ static library to interface with the 7-zip shared libraries.
 * Copyright (c) 2014-2022 Riccardo Ostani - All Rights Reserved.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef BITARCHIVEREADER_HPP
#define BITARCHIVEREADER_HPP

#include "bitabstractarchiveopener.hpp"
#include "bitarchiveiteminfo.hpp"
#include "bitinputarchive.hpp"

struct IInArchive;
struct IOutArchive;
struct IArchiveExtractCallback;

namespace bit7z {
/**
 * @brief The BitArchiveReader class allows to read metadata of archives, as well as extract them.
 */
class BitArchiveReader final : public BitAbstractArchiveOpener, public BitInputArchive {
    public:
        /**
         * @brief Constructs a BitArchiveReader object, opening the input file archive.
         *
         * @note When bit7z is compiled using the BIT7Z_AUTO_FORMAT macro define, the format
         * argument has default value BitFormat::Auto (automatic format detection of the input archive).
         * On the contrary, when BIT7Z_AUTO_FORMAT is not defined (i.e., no auto format detection available)
         * the format argument must be specified.
         *
         * @param lib       the 7z library used.
         * @param in_file   the input archive file path.
         * @param format    the input archive format.
         * @param password  the password needed for opening the input archive.
         */
        BitArchiveReader( const Bit7zLibrary& lib,
                          const tstring& in_file,
                          const BitInFormat& format BIT7Z_DEFAULT_FORMAT,
                          const tstring& password = {} );

        /**
         * @brief Constructs a BitArchiveReader object, opening the archive in the input buffer.
         *
         * @note When bit7z is compiled using the BIT7Z_AUTO_FORMAT macro define, the format
         * argument has default value BitFormat::Auto (automatic format detection of the input archive).
         * On the contrary, when BIT7Z_AUTO_FORMAT is not defined (i.e., no auto format detection available)
         * the format argument must be specified.
         *
         * @param lib       the 7z library used.
         * @param in_buffer the input buffer containing the archive.
         * @param format    the input archive format.
         * @param password  the password needed for opening the input archive.
         */
        BitArchiveReader( const Bit7zLibrary& lib,
                          const vector< byte_t >& in_buffer,
                          const BitInFormat& format BIT7Z_DEFAULT_FORMAT,
                          const tstring& password = {} );

        /**
         * @brief Constructs a BitArchiveReader object, opening the archive from the standard input stream.
         *
         * @note When bit7z is compiled using the BIT7Z_AUTO_FORMAT macro define, the format
         * argument has default value BitFormat::Auto (automatic format detection of the input archive).
         * On the contrary, when BIT7Z_AUTO_FORMAT is not defined (i.e., no auto format detection available)
         * the format argument must be specified.
         *
         * @param lib       the 7z library used.
         * @param in_stream the standard input stream of the archive.
         * @param format    the input archive format.
         * @param password  the password needed for opening the input archive.
         */
        BitArchiveReader( const Bit7zLibrary& lib,
                          std::istream& in_stream,
                          const BitInFormat& format BIT7Z_DEFAULT_FORMAT,
                          const tstring& password = {} );

        BitArchiveReader( const BitArchiveReader& ) = delete;

        BitArchiveReader( BitArchiveReader&& ) = delete;

        BitArchiveReader& operator=( const BitArchiveReader& ) = delete;

        BitArchiveReader& operator=( BitArchiveReader&& ) = delete;

        /**
         * @brief BitArchiveReader destructor.
         *
         * @note It releases the input archive file.
         */
        ~BitArchiveReader() override = default;

        /**
         * @return a map of all the available (i.e., non-empty) archive properties and their respective values.
         */
        BIT7Z_NODISCARD map< BitProperty, BitPropVariant > archiveProperties() const;

        /**
         * @return a vector of all the archive items as BitArchiveItem objects.
         */
        BIT7Z_NODISCARD vector< BitArchiveItemInfo > items() const;

        /**
         * @return the number of folders contained in the archive.
         */
        BIT7Z_NODISCARD uint32_t foldersCount() const;

        /**
         * @return the number of files contained in the archive.
         */
        BIT7Z_NODISCARD uint32_t filesCount() const;

        /**
         * @return the total uncompressed size of the archive content.
         */
        BIT7Z_NODISCARD uint64_t size() const;

        /**
         * @return the total compressed size of the archive content.
         */
        BIT7Z_NODISCARD uint64_t packSize() const;

        /**
         * @return true if and only if the archive has at least one encrypted item.
         */
        BIT7Z_NODISCARD bool hasEncryptedItems() const;

        /**
         * @return the number of volumes composing the archive.
         */
        BIT7Z_NODISCARD uint32_t volumesCount() const;

        /**
         * @return true if and only if the archive is composed by multiple volumes.
         */
        BIT7Z_NODISCARD bool isMultiVolume() const;

        /**
         * @return true if and only if the archive was created using solid compression.
         */
        BIT7Z_NODISCARD bool isSolid() const;
};
}  // namespace bit7z

#endif // BITARCHIVEREADER_HPP
