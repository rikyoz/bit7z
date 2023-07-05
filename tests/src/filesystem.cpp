// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

/*
 * bit7z - A C++ static library to interface with the 7-zip shared libraries.
 * Copyright (c) 2014-2022 Riccardo Ostani - All Rights Reserved.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "filesystem.hpp"

namespace bit7z { // NOLINT(modernize-concat-nested-namespaces)
namespace test {
namespace filesystem {

const FilesystemItemInfo italy{ BIT7Z_STRING( "italy.svg" ),
                                BIT7Z_STRING( "svg" ),
                                false,
                                267,
                                0x2214F4E9 };

const FilesystemItemInfo lorem_ipsum{ BIT7Z_STRING( "Lorem Ipsum.pdf" ),
                                      BIT7Z_STRING( "pdf" ),
                                      false,
                                      38170,
                                      0xC0629B8E };

const FilesystemItemInfo noext{ BIT7Z_STRING( "noext" ),
                                BIT7Z_STRING( "" ),
                                false,
                                34,
                                0xDE536DF9 };

const FilesystemItemInfo dot_folder{ BIT7Z_STRING( "dot.folder" ),
                                     BIT7Z_STRING( "" ),
                                     true,
                                     0,
                                     0 };

const FilesystemItemInfo hello_json{ BIT7Z_STRING( "hello.json" ),
                                     BIT7Z_STRING( "json" ),
                                     false,
                                     29,
                                     0x794FDB10 };

const FilesystemItemInfo empty_folder{ BIT7Z_STRING( "empty" ),
                                       BIT7Z_STRING( "" ),
                                       true,
                                       0,
                                       0 };

const FilesystemItemInfo folder{ BIT7Z_STRING( "folder" ),
                                 BIT7Z_STRING( "" ),
                                 true,
                                 0,
                                 0 };

const FilesystemItemInfo subfolder{ BIT7Z_STRING( "subfolder" ),
                                    BIT7Z_STRING( "" ),
                                    true,
                                    0,
                                    0 };

const FilesystemItemInfo subfolder2{ BIT7Z_STRING( "subfolder2" ),
                                     BIT7Z_STRING( "" ),
                                     true,
                                     0,
                                     0 };

const FilesystemItemInfo homework{ BIT7Z_STRING( "homework.doc" ),
                                   BIT7Z_STRING( "doc" ),
                                   false,
                                   31232,
                                   0x1734526A };

const FilesystemItemInfo quick_brown{ BIT7Z_STRING( "The quick brown fox.pdf" ),
                                      BIT7Z_STRING( "pdf" ),
                                      false,
                                      45933,
                                      0x3A4DCE2A };

const FilesystemItemInfo frequency{ BIT7Z_STRING( "frequency.xlsx" ),
                                    BIT7Z_STRING( "xlsx" ),
                                    false,
                                    20803,
                                    0xA140E471 };

const FilesystemItemInfo clouds{ BIT7Z_STRING( "clouds.jpg" ),
                                 BIT7Z_STRING( "jpg" ),
                                 false,
                                 478883,
                                 0x515D4B66 };

auto single_file_content() -> const ArchiveContent& {
    static const ArchiveContent instance{ 1, clouds.size, { { clouds, clouds.name } } };
    return instance;
}

auto multiple_files_content() -> const ArchiveContent& {
    static const ArchiveContent instance{ 2,
                                          italy.size + lorem_ipsum.size,
                                          { { italy, italy.name },
                                            { lorem_ipsum, lorem_ipsum.name } } };
    return instance;
}

auto multiple_items_content() -> const ArchiveContent& {
    static const ArchiveContent instance{ 8,
                                          615351,
                                          { { italy, "italy.svg" },
                                            { lorem_ipsum, "Lorem Ipsum.pdf" },
                                            { noext, "noext" },
                                            { dot_folder, "dot.folder" },
                                            { hello_json, "dot.folder/hello.json" },
                                            { empty_folder, "empty" },
                                            { folder, "folder" },
                                            { clouds, "folder/clouds.jpg" },
                                            { subfolder, "folder/subfolder" },
                                            { subfolder2, "folder/subfolder2" },
                                            { frequency, "folder/subfolder2/frequency.xlsx" },
                                            { homework, "folder/subfolder2/homework.doc" },
                                            { quick_brown, "folder/subfolder2/The quick brown fox.pdf" } } };
    return instance;
}

auto encrypted_content() -> const ArchiveContent& {
    static const ArchiveContent instance{ 8,
                                          615351,
                                          { { italy, "italy.svg", true },
                                            { lorem_ipsum, "Lorem Ipsum.pdf", true },
                                            { noext, "noext", true },
                                            { dot_folder, "dot.folder" },
                                            { hello_json, "dot.folder/hello.json", true },
                                            { empty_folder, "empty" },
                                            { folder, "folder" },
                                            { clouds, "folder/clouds.jpg", true },
                                            { subfolder, "folder/subfolder" },
                                            { subfolder2, "folder/subfolder2" },
                                            { frequency, "folder/subfolder2/frequency.xlsx", true },
                                            { homework, "folder/subfolder2/homework.doc", true },
                                            { quick_brown, "folder/subfolder2/The quick brown fox.pdf", true } } };
    return instance;
}

auto empty_content() -> const ArchiveContent& {
    static const ArchiveContent instance{ 0, 0, {} };
    return instance;
}

} // namespace filesystem
} // namespace test
} // namespace bit7z