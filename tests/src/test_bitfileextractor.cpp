// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

/*
 * bit7z - A C++ static library to interface with the 7-zip shared libraries.
 * Copyright (c) 2014-2023 Riccardo Ostani - All Rights Reserved.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include <catch2/catch.hpp>

#include <map>

#include <vector>

#include <random>

#include "utils/shared_lib.hpp"

#include <bit7z/bit7zlibrary.hpp>

#include <bit7z/bitfileextractor.hpp>

#include <bit7z/bitfilecompressor.hpp>

using namespace bit7z;

namespace {
    auto createArchive(Bit7zLibrary& lib, const std::map< tstring, std::vector< byte_t > >& filenamesAndContents) -> std::vector< byte_t > {
        BitFileCompressor compressor{lib, BitFormat::SevenZip};
        compressor.setCompressionLevel(BitCompressionLevel::Fastest);
        BitOutputArchive actualCompressor{compressor};

        for (auto& filenameAndContent : filenamesAndContents) {
            const std::string& filename = filenameAndContent.first;
            const std::vector< byte_t >& content = filenameAndContent.second;
            actualCompressor.addFile(content, filename);
        }

        std::vector< byte_t > buffer;
        actualCompressor.compressTo(buffer);
        return buffer;
    }

    std::vector<byte_t> generateBuffer(std::size_t length) {
        std::random_device dev{};
        std::default_random_engine engine{dev()};
        std::uniform_int_distribution<unsigned> distribution{0, 255};

        std::vector<byte_t> buffer;
        buffer.reserve(length);
        for (unsigned i=0; i < length; ++i) {
            buffer.push_back(distribution(engine));
        }

        return buffer;
    }

    std::vector<byte_t> tstringToVector(tstring input) {
        std::vector<byte_t> result;
        result.reserve(input.size());
        std::copy(input.begin(), input.end(), std::back_inserter(result));
        return result;
    }

    struct PreExtractionData {
        std::map<uint32_t, std::vector<byte_t>> buffers;
        std::map<uint32_t, ByteSpan> bufferViews;
        std::map<tstring, int32_t> fileNameToIndex;
    };

    PreExtractionData prepareExtractionData(BitInputArchive& archive) {
        PreExtractionData result;
        auto const numberOfItems = archive.itemsCount();
        for (uint32_t index =0; index < numberOfItems; ++index) {
            auto item = archive.itemAt(index);
            if (item.isDir()) continue;
            const auto neededSizeToExtract = item.size();
            if (neededSizeToExtract == 0) continue;
            result.buffers.emplace(index, std::vector<byte_t>(neededSizeToExtract));
            result.bufferViews.emplace(index, ByteSpan{result.buffers[index].data(), result.buffers[index].size()});
            result.fileNameToIndex.emplace(item.name(), index);
        }

        return result;
    }
}

TEST_CASE( "BitFileExtractor", "extract to map of Bitspans" ) {
    Bit7zLibrary lib{ test::sevenzip_lib_path() };

    std::map< tstring, std::vector< byte_t > > files{
        {tstring{"simple.txt"}, tstringToVector("simple file content in simple.txt")},
        {tstring{"empty.bar"}, std::vector<byte_t>{}},
        {tstring{"medium_file.buff"}, generateBuffer(5*1024*1024)},
    };

    auto archiveAsBuffer = createArchive(lib, files);

    const BitFileExtractor extractor{lib, BitFormat::SevenZip};
    BitInputArchive actualExtractor{extractor, archiveAsBuffer};

    auto extractionData = prepareExtractionData(actualExtractor);

    auto archiveContains = [&extractionData](const tstring& fileName) {
        return extractionData.fileNameToIndex.find(fileName) != extractionData.fileNameToIndex.end();
    };

    REQUIRE(archiveContains("simple.txt"));
    CHECK(!archiveContains("empty.bar"));
    REQUIRE(archiveContains("medium_file.buff"));

    actualExtractor.extractTo(extractionData.bufferViews);

    auto extractedFileContent = [&extractionData](const tstring& fileName) {
        const auto idx = extractionData.fileNameToIndex.at(fileName);
        return extractionData.buffers.at(idx);
    };

    CHECK(extractedFileContent("simple.txt") == files["simple.txt"]);
    CHECK(extractedFileContent("medium_file.buff") == files["medium_file.buff"]);
}

TEST_CASE( "BitFileExtractor: TODO", "[bitfileextractor]" ) {
    const Bit7zLibrary lib{ test::sevenzip_lib_path() };

    const BitFileExtractor extractor{lib, BitFormat::SevenZip};
    REQUIRE( extractor.extractionFormat() == BitFormat::SevenZip ); // Just a placeholder test.
}