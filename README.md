<h1 align="center">bit7z</h1>

<h3 align="center">A C++ static library offering a clean and simple interface to the 7-zip shared libraries</h3>

<!-- navbar -->
<p align="center">
  <a href="#-supported-features" title="List of Features Supported by the Library">Supported Features</a> •
  <a href="#-getting-started-library-usage" title="Basic Source Code Examples">Getting Started</a> •
  <a href="#-download" title="Download Pre-compiled Packages">Download</a> •
  <a href="#-requirements" title="Usage Requirements">Requirements</a> •
  <a href="#%EF%B8%8F-building-and-using-bit7z" title="Building the Library">Building & Using</a> •
  <a href="#%EF%B8%8F-donate" title="Support the Project">Donate</a> •
  <a href="#-license" title="Project License">License</a>
</p>
<!-- navbar -->

<div align="center">
  <a href="https://github.com/rikyoz/bit7z/releases" title="Latest Stable GitHub Release"><img src="https://img.shields.io/github/release/rikyoz/bit7z/all.svg?style=flat&logo=github&logoColor=white&colorB=blue&label=" alt="GitHub release"></a>&thinsp;<img src="https://img.shields.io/badge/-C++14/17-3F63B3.svg?style=flat&logo=C%2B%2B&logoColor=white" alt="C++14/17" title="C++ Standards Used: C++14/17">&thinsp;<img src="https://img.shields.io/badge/-Windows-6E46A2.svg?style=flat&logo=windows-11&logoColor=white" alt="Windows" title="Supported Platform: Windows">&thinsp;<img src="https://img.shields.io/badge/-Linux-9C2A91.svg?style=flat&logo=linux&logoColor=white" alt="Linux" title="Supported Platform: Linux">&thinsp;<img src="https://img.shields.io/badge/-macOS-red.svg?style=flat&logo=apple&logoColor=white" alt="macOS" title="Supported Platform: macOS">&thinsp;<img src="https://img.shields.io/badge/-x86%20&middot;%20x64-orange.svg?style=flat&logo=data:image/svg+xml;base64,PHN2ZyB4bWxucz0iaHR0cDovL3d3dy53My5vcmcvMjAwMC9zdmciIHhtbDpzcGFjZT0icHJlc2VydmUiIHZpZXdCb3g9IjAgMCA5NDIgOTQyIj48cGF0aCBmaWxsPSIjZmZmIiBkPSJNNTc5LjEgODk0YTQ4IDQ4IDAgMCAwIDk2IDB2LTc3LjVoLTk1LjlWODk0aC0uMXpNNTc5LjEgNDh2NzcuNUg2NzVWNDhhNDggNDggMCAwIDAtOTUuOSAwek00MjMgNDh2NzcuNWg5NlY0OGE0OCA0OCAwIDAgMC05NiAwek00MjMgODk0YTQ4IDQ4IDAgMCAwIDk2IDB2LTc3LjVoLTk2Vjg5NHpNMjY3IDQ4djc3LjVoOTUuOVY0OGE0OCA0OCAwIDAgMC05NS45IDB6TTI2NyA4OTRhNDggNDggMCAwIDAgOTYgMHYtNzcuNWgtOTZWODk0ek0wIDYyN2E0OCA0OCAwIDAgMCA0OCA0OGg3Ny41di05NS45SDQ4QTQ4IDQ4IDAgMCAwIDAgNjI3ek04OTQgNTc5LjFoLTc3LjVWNjc1SDg5NGE0OCA0OCAwIDAgMCAwLTk1Ljl6TTAgNDcxYTQ4IDQ4IDAgMCAwIDQ4IDQ4aDc3LjV2LTk2SDQ4YTQ4IDQ4IDAgMCAwLTQ4IDQ4ek04OTQgNDIzaC03Ny41djk2SDg5NGE0OCA0OCAwIDAgMCAwLTk2ek0wIDMxNWE0OCA0OCAwIDAgMCA0OCA0OGg3Ny41di05Nkg0OGE0OCA0OCAwIDAgMC00OCA0OHpNODk0IDI2N2gtNzcuNXY5NS45SDg5NGE0OCA0OCAwIDAgMCAwLTk1Ljl6TTE3MS42IDcyMC40YTUwIDUwIDAgMCAwIDUwIDUwaDQ5OC44YTUwIDUwIDAgMCAwIDUwLTUwVjIyMS42YTUwIDUwIDAgMCAwLTUwLTUwSDIyMS42YTUwIDUwIDAgMCAwLTUwIDUwdjQ5OC44eiIvPjwvc3ZnPg==" alt="x86, x64" title="Supported CPU Architectures: x86, x64">&thinsp;<a href="#%EF%B8%8F-donate" title="Donate"><img src="https://img.shields.io/badge/-donate-yellow.svg?style=flat&logo=paypal&logoColor=white" alt="donate"></a>&thinsp;<a href="https://github.com/rikyoz/bit7z/wiki" title="Project Documentation"><img src="https://img.shields.io/badge/-docs-green.svg?style=flat&logo=data:image/svg+xml;base64,PHN2ZyB4bWxucz0iaHR0cDovL3d3dy53My5vcmcvMjAwMC9zdmciIGRhdGEtbmFtZT0iTGF5ZXIgMSIgdmlld0JveD0iMCAwIDEwNS4zIDEyMi45Ij48cGF0aCBmaWxsLXJ1bGU9ImV2ZW5vZGQiIGZpbGw9IiNmZmYiIGQ9Ik0xNy41IDBIMTAydjk0LjJjLS4xIDIuNy0zLjUgMi43LTcuMiAyLjZIMTYuM2E5LjIgOS4yIDAgMCAwIDAgMTguNEg5OHYtOS44aDcuMlYxMThhNC4yIDQuMiAwIDAgMS00LjEgNC4xSDE2LjZDNy41IDEyNS41IDAgMTE4IDAgMTA4LjhWMTcuNUExNy42IDE3LjYgMCAwIDEgMTcuNSAwWm0tMS4zIDEwOGg3NS4yYTEuNCAxLjQgMCAwIDEgMS40IDEuM3YuOGExLjQgMS40IDAgMCAxLTEuNCAxLjRIMTYuMmExLjQgMS40IDAgMCAxLTEuMy0xLjR2LS44YTEuNCAxLjQgMCAwIDEgMS4zLTEuNFptMC03LjJoNzUuMmExLjQgMS40IDAgMCAxIDEuNCAxLjR2LjhhMS40IDEuNCAwIDAgMS0xLjQgMS40SDE2LjJBMS40IDEuNCAwIDAgMSAxNSAxMDN2LS44YTEuNCAxLjQgMCAwIDEgMS4zLTEuNFoiLz48L3N2Zz4=" alt="docs"></a>&thinsp;<a href="https://ci.appveyor.com/project/rikyoz/bit7z" title="AppVeyor CI Build Status"><img src="https://img.shields.io/appveyor/ci/rikyoz/bit7z.svg?style=flat&logo=appveyor&logoColor=white&label=" alt="Build status"></a>
  <br>
  <img src="https://img.shields.io/badge/MSVC%202015+-flag.svg?color=555555&style=flat&logo=visual%20studio&logoColor=white" alt="MSVC 2015+" title="Supported Windows Compiler: MSVC 2015 or later">&thinsp;<img src="https://img.shields.io/badge/MinGW%206.4+%20-flag.svg?color=555555&style=flat&logo=windows&logoColor=white" alt="MinGW 6.4+" title="Supported Windows Compiler: MinGW 6.4 or later">&thinsp;<img src="https://img.shields.io/badge/GCC%204.9+-flag.svg?color=555555&style=flat&logo=gnu&logoColor=white" alt="GCC 4.9+" title="Supported Unix Compiler: GCC 4.9 or later">&thinsp;<img src="https://img.shields.io/badge/Clang%203.5+-flag.svg?color=555555&style=flat&logo=llvm&logoColor=white" alt="Clang 3.5+" title="Supported Unix Compiler: Clang 3.5 or later">&thinsp;<img alt="CodeFactor Grade" src="https://img.shields.io/codefactor/grade/github/rikyoz/bit7z?label=Code%20Quality&logo=codefactor&logoColor=white">&thinsp;<a href="https://github.com/rikyoz/bit7z/blob/master/LICENSE" title="Project License: MPLv2"><img src="https://img.shields.io/badge/-MPL--2.0-lightgrey.svg?style=flat&logo=data:image/svg+xml;base64,PHN2ZyB4bWxucz0iaHR0cDovL3d3dy53My5vcmcvMjAwMC9zdmciIGZpbGwtcnVsZT0iZXZlbm9kZCIgaW1hZ2UtcmVuZGVyaW5nPSJvcHRpbWl6ZVF1YWxpdHkiIHNoYXBlLXJlbmRlcmluZz0iZ2VvbWV0cmljUHJlY2lzaW9uIiB2aWV3Qm94PSIwIDAgNTEyIDQzMCIgeG1sbnM6dj0iaHR0cHM6Ly92ZWN0YS5pby9uYW5vIj48cGF0aCBmaWxsPSIjZmZmIiBkPSJNMzUgMGg0NDJjMTkgMCAzNSAxNiAzNSAzNXYzMDBjMCAxOS0xNiAzNS0zNSAzNWgtNjlsLTExLTIyaDgyYzUgMCAxMC00IDEwLTEwVjk3SDI1djI0MWMwIDYgNCAxMCA5IDEwaDgxbC05IDE4LTIgNEgzNWMtMTkgMC0zNS0xNi0zNS0zNVYzNUMwIDE2IDE2IDAgMzUgMHptMjI5IDE1MWM1IDIgOSA1IDEzIDkgOSA3IDEwIDkgMjMgOWwxNCAyYzggMyAxMyAxMCAxNSAxOCAyIDcgMSAxNCA0IDIxIDIgNyA3IDEyIDExIDE4IDMgNSA0IDkgNSAxM2EyNSAyNSAwIDAgMS03IDI0bC05IDlhMjYgMjYgMCAwIDAtMiAxMWMwIDUgMCAxMC0zIDE3YTMyIDMyIDAgMCAxLTIwIDE2Yy02IDItMTQgMC0xOCAyaC0xYy04IDMtMTUgMTItMjMgMTVhMzEgMzEgMCAwIDEtMTAgMmwtOS0yaC0xYy00LTEtOC00LTEyLTgtMy0zLTctNi0xMi04aC03bC0xMS0xYy01LTEtOC0zLTEyLTZhMzMgMzMgMCAwIDEtOC0xMGMtMy03LTMtMTItMy0xNyAwLTkgMC0xMS03LTE2LTgtOC0xNC0xNi0xMi0yOCAyLTYgNS0xMiAxMS0yMGEzNyAzNyAwIDAgMCA2LTExYzMtNiAyLTEzIDMtMjAgMi04IDctMTUgMTUtMThsMTUtM2MxMiAwIDE0LTIgMjMtOWE0MSA0MSAwIDAgMSAxMy05IDI3IDI3IDAgMCAxIDE2IDB6bTg3IDI1N2wtMjQtNC0xMSAyMWgtMWMtNSA2LTkgNi0xMyAzcy03LTExLTktMTV2LTJsLTI1LTQ3YTMgMyAwIDAgMSAxLTRsOS01IDEwLTlhMyAzIDAgMCAxIDIgMGM3IDAgMTQtMSAxOS0zIDYtMiAxMS01IDE2LTExaDRsMSAxIDI5IDU1YzIgNSA1IDExIDIgMTYtMSAzLTQgNS05IDRoLTF6bS0xNjYtNGwtMjQgNGgtMWMtNSAxLTgtMS05LTRsLTEtOCAzLTggMjktNTYgMS0xYzEtMSAzIDAgNCAxIDQgNiAxMCA5IDE1IDExIDYgMiAxMyAzIDIwIDJsMiAxIDEwIDkgOSA1YTMgMyAwIDAgMSAxIDRsLTI1IDQ3djFhMzkgMzkgMCAwIDEtOSAxNmMtNCAzLTggMy0xMy0zaC0xbC0xMS0yMXptMTAwLTIwM2E1MSA1MSAwIDAgMC04MCA0MiA1MSA1MSAwIDEgMCAxMDIgMCA1MiA1MiAwIDAgMC03LTI2IDUxIDUxIDAgMCAwLTE1LTE2ek00NDQgMzlhMTcgMTcgMCAxIDEgMCAzNCAxNyAxNyAwIDAgMSAwLTM0em0tMTE2IDBhMTcgMTcgMCAxIDEgMCAzNCAxNyAxNyAwIDAgMSAwLTM0em01OCAwYTE3IDE3IDAgMSAxIDAgMzQgMTcgMTcgMCAwIDEgMC0zNHoiLz48L3N2Zz4=" alt="License"></a>
</div>

## ⚡️ Introduction

**bit7z** is a _cross-platform_ C++ static library that allows the _compression/extraction of archive files_ through a _clean_ and _simple_ wrapper interface to the dynamic libraries from the [7-zip](https://www.7-zip.org/ "7-zip Project Homepage") project.<br/>
It supports compression and extraction to and from the filesystem or the memory, reading archives metadata, updating existing ones, creating multi-volume archives, operation progress callbacks, and many other functionalities.

## 🎯 Supported Features

+ **Compression** using the following archive formats: **7z**, XZ, **BZIP2**, **GZIP**, TAR, **ZIP**, and WIM.
+ **Extraction** of many archive formats: **7z**, AR, ARJ, **BZIP2**, CAB, CHM, CPIO, CramFS, DEB, DMG, EXT, FAT, GPT, **GZIP**, HFS, HXS, IHEX, ISO, LZH, LZMA, MBR, MSI, NSIS, NTFS, QCOW2, **RAR**, **RAR5**, RPM, SquashFS, TAR, UDF, UEFI, VDI, VHD, VMDK, WIM, XAR, XZ, Z, and **ZIP**.
+ **Reading metadata** of archives and their content.
+ **Testing** archives for errors.
+ **Updating** existing file archives with new files.
+ **Renaming**, **updating**, or **deleting** old items in existing file archives.
+ **Compression and extraction _to and from_ memory** and **C++ standard streams**.
+ Compression using **custom path aliases** for the items in the output archives.
+ **Selective extraction** of only specified files/folders **using wildcards** and **regular expressions**.
+ Creation of **encrypted archives** (strong AES-256 encryption &mdash; only for 7z and ZIP formats).
+ **Archive header encryption** (only for 7z format).
+ Possibility to choose the **compression level** (if supported by the archive format), the **compression method** ([supported methods](https://github.com/rikyoz/bit7z/wiki/Advanced-Usage#compression-methods "Wiki page on bit7z's supported compression methods")), the **dictionary size**, and the **word size**.
+ **Automatic input archive format detection**.
+ **Solid archives** (only for 7z).
+ **Multi-volume archives**.
+ **Operation callbacks** for obtaining real-time information about ongoing operations.
+ **Canceling** or **pausing** the current operation.

### Notes

The presence or not of some of the above features depends on the particular shared library used along with bit7z.<br/>
For example, 7z.dll should support all these features, 7za.dll should work only with the 7z file format, and 7zxa.dll can only extract 7z files. For more information about the 7-zip DLLs, please check this [wiki page](https://github.com/rikyoz/bit7z/wiki/7z-DLLs).

In the end, some other features (e.g., _automatic format detection_ and _selective extraction using regular expressions_) are disabled by default, and macro definitions must be used during compilation to have them available ([wiki](https://github.com/rikyoz/bit7z/wiki/Building-the-library)).

## 🔥 Getting Started (Library Usage)

Below are a few examples that show how to use some of the main features of bit7z.

### 📂 Extracting files from an archive

```cpp
#include <bit7z/bitfileextractor.hpp>

try { // bit7z classes can throw BitException objects
    using namespace bit7z;

    Bit7zLibrary lib{ "7za.dll" };
    BitFileExtractor extractor{ lib, BitFormat::SevenZip };

    // Extracting a simple archive
    extractor.extract( "path/to/archive.7z", "out/dir/" );

    // Extracting a specific file
    extractor.extractMatching( "path/to/archive.7z", "file.pdf", "out/dir/" );

    // Extracting the first file of an archive to a buffer
    std::vector< byte_t > buffer;
    extractor.extract( "path/to/archive.7z", buffer );

    // Extracting an encrypted archive
    extractor.setPassword( "password" );
    extractor.extract( "path/to/another/archive.7z", "out/dir/" );
} catch ( const bit7z::BitException& ex ) { /* Do something with ex.what()...*/ }
```

Alternatively, if you only need to work on a single archive:

```cpp
#include <bit7z/bitarchivereader.hpp>

try { // bit7z classes can throw BitException objects
    using namespace bit7z;

    Bit7zLibrary lib{ "7z.dll" };

    // Opening the archive
    BitArchiveReader reader{lib, "path/to/archive.gz", BitFormat::GZip };

    // Testing the archive
    reader.test();

    // Extracting the archive
    reader.extract( "out/dir/" );
} catch ( const bit7z::BitException& ex ) { /* Do something with ex.what()...*/ }
```

### 💼 Compressing files into an archive

```cpp
#include <bit7z/bitfilecompressor.hpp>

try { // bit7z classes can throw BitException objects
    using namespace bit7z;

    Bit7zLibrary lib{ "7z.dll" };
    BitFileCompressor compressor{ lib, BitFormat::Zip };

    std::vector< std::string > files = { "path/to/file1.jpg", "path/to/file2.pdf" };

    // Creating a simple zip archive
    compressor.compress( files, "output_archive.zip" );

    // Creating a zip archive with a custom directory structure
    std::map< std::string, std::string > files_map = {
        { "path/to/file1.jpg", "alias/path/file1.jpg" },
        { "path/to/file2.pdf", "alias/path/file2.pdf" }
    };
    compressor.compress( files_map, "output_archive2.zip" );

    // Compressing a directory
    compressor.compressDirectory( "dir/path/", "dir_archive.zip" );

    // Creating an encrypted zip archive of two files
    compressor.setPassword( "password" );
    compressor.compressFiles( files, "protected_archive.zip" );

    // Updating an existing zip archive
    compressor.setUpdateMode( UpdateMode::Append );
    compressor.compressFiles( files, "existing_archive.zip" );

    // Compressing a single file into a buffer
    std::vector< bit7z::byte_t > buffer;
    BitFileCompressor compressor2{ lib, BitFormat::BZip2 };
    compressor2.compressFile( files[0], buffer );
} catch ( const bit7z::BitException& ex ) { /* Do something with ex.what()...*/ }
```

Alternatively, if you only need to work on a single archive:

```cpp
#include <bit7z/bitarchivewriter.hpp>

try { // bit7z classes can throw BitException objects
    using namespace bit7z;

    Bit7zLibrary lib{ "7z.dll" };
    BitArchiveWriter writer{lib, BitFormat::SevenZip };

    // Adding the items to be compressed (no compression is performed here)
    writer.addFile( "path/to/file.txt" );
    writer.addDirectory( "path/to/dir/" );

    // Compressing the added items to the output archive
    writer.compressTo( "output.7z" );
} catch ( const bit7z::BitException& ex ) { /* Do something with ex.what()...*/ }
```

### 📑 Reading archive metadata

```cpp
#include <bit7z/bitarchivereader.hpp>

try { // bit7z classes can throw BitException objects
    using namespace bit7z;

    Bit7zLibrary lib{ "7za.dll" };
    BitArchiveReader arc{ lib, "archive.7z", BitFormat::SevenZip };

    // Printing archive metadata
    std::cout << "Archive properties" << std::endl;
    std::cout << "  Items count: "   << arc.itemsCount() << std::endl;
    std::cout << "  Folders count: " << arc.foldersCount() << std::endl;
    std::cout << "  Files count: "   << arc.filesCount() << std::endl;
    std::cout << "  Size: "          << arc.size() << std::endl;
    std::cout << "  Packed size: "   << arc.packSize() << std::endl;
    std::cout << std::endl;

    // Printing the metadata of the archived items
    std::cout << "Archived items";
    auto arc_items = arc.items();
    for ( auto& item : arc_items ) {
        std::cout << std::endl;
        std::cout << "  Item index: "   << item.index() << std::endl;
        std::cout << "    Name: "        << item.name() << std::endl;
        std::cout << "    Extension: "   << item.extension() << std::endl;
        std::cout << "    Path: "        << item.path() << std::endl;
        std::cout << "    IsDir: "       << item.isDir() << std::endl;
        std::cout << "    Size: "        << item.size() << std::endl;
        std::cout << "    Packed size: " << item.packSize() << std::endl;
        std::cout << "    CRC: "         << item.crc() << std::endl;
    }
} catch ( const bit7z::BitException& ex ) { /* Do something with ex.what()...*/ }
```

A complete _**API reference**_ is available in the [wiki](https://github.com/rikyoz/bit7z/wiki/) section.

## 🚀 Upgrading from bit7z v3 to v4

<details>
  <summary>Expand for more details!</summary>

The newest bit7z v4 introduced some significant breaking changes to the API. In particular:

+ By default, the project now follows the [UTF-8 Everywhere Manifesto](http://utf8everywhere.org/):
  + The default string type is `std::string` (instead of `std::wstring`), so users can use the library in cross-platform projects more easily (v4 introduced Linux/macOS support too).
  + Input `std::string`s will be considered as UTF-8 encoded.
  + You can still achieve the old behavior on Windows using the `-DBIT7Z_USE_NATIVE_STRING` CMake option.
+ The old `BitExtractor` class is now called `BitFileExtractor`.
  + Now `BitExtractor` is just the name of a template class for all the extraction classes.
+ The `ProgressCallback` now must return a `bool` value indicating whether the current operation can continue (`true`) or not (`false`).

</details>

## 💾 Download

<div align="center">
<a href="https://github.com/rikyoz/bit7z/releases">
<img alt="GitHub Latest Release" src="https://img.shields.io/github/v/release/rikyoz/bit7z?include_prereleases&sort=semver&label=Latest%20Release&style=social" height='36' style='border:0;height:36px;'/></a>
&emsp;&emsp;
<a href="https://github.com/rikyoz/bit7z/releases/latest">
<img alt="GitHub Stable Release" src="https://img.shields.io/github/v/release/rikyoz/bit7z?label=Stable%20Release&style=social" height='36' style='border:0;height:36px;'/></a>
<br/>
<a href="https://github.com/rikyoz/bit7z/releases">
<img alt="GitHub All Releases" src="https://img.shields.io/github/downloads/rikyoz/bit7z/total.svg?style=popout&label=Total%20Downloads&logo=icloud&logoColor=white"/></a>
</div>

Each released package contains:

+ A _pre-compiled version_ of bit7z (both in _debug_ and _release_ mode);
+ The _public API headers_ needed to use the library in your program;

Packages are available for both _x86_ and _x64_ architectures.

You can also clone/download this repository and build the library by yourself (please, read the [wiki](https://github.com/rikyoz/bit7z/wiki/Building-the-library)).

## 🧰 Requirements

+ **Operating System:** Windows, Linux, macOS[^1].
+ **Architecture:** x86, x86_64.
+ **Compiler:** MSVC 2015 or later[^2], MinGW v6.4 or later, GCC v4.9 or later, Clang 3.5 or later.
+ **Shared Library:** a 7-zip `.dll` library on Windows, a 7-zip/p7zip `.so` library on Unix[^3].

[^1]: On Windows, you should link your program _also_ with _oleaut32_ (e.g., `-lbit7z -loleaut32`).<br/> On Linux and macOS, you should link your program _also_ with _dl_ (e.g., `-lbit7z -ldl`).<br/> If you are using the library via CMake, these dependencies will be linked automatically to your project.

[^2]: MSVC 2010 was supported until v2.x, MSVC 2012/2013 until v3.x.

[^3]: bit7z doesn't ship with the 7-zip shared libraries. You can build them from the source code available at [7-zip.org](http://www.7-zip.org/).

## ⚙️ Building and using bit7z

For building the library:

```bash
cd <bit7z folder>
mkdir build && cd build
cmake ../ -DCMAKE_BUILD_TYPE=Release
cmake --build . -j --config Release
```

A more detailed guide on how to build this library is available [here](https://github.com/rikyoz/bit7z/wiki/Building-the-library).

You can directly integrate the library into your project via CMake:

+ Download bit7z and copy it into a sub-directory of your project (e.g., `third_party`), or add it as a git submodule of your repository.
+ You can then use the command `add_subdirectory()` in your `CMakeLists.txt` to include bit7z.
+ Finally, add the `bit7z64` target (just `bit7z` on x86) using the `target_link_libraries()` command.

For example:

```cmake
add_subdirectory( ${CMAKE_SOURCE_DIR}/third_party/bit7z )
target_link_libraries( ${TARGET_NAME} PRIVATE bit7z64 ) # or bit7z on x86
```

## ☕️ Donate

If you have found this project helpful, please consider supporting me with a small donation so that I can keep improving it!
Thank you! :pray:

<div align="center">
<a href='https://github.com/sponsors/rikyoz' target='_blank' title="Support the Maintainer via GitHub Sponsors"><img height='24' style='border:0px;height:24px;' src='https://img.shields.io/badge/-Sponsor%20the%20Maintainer-fafbfc?logo=GitHub%20Sponsors' border='0' alt='Sponsor me on GitHub' /></a>&thinsp;<a href='https://ko-fi.com/G2G1LS1P' target='_blank' title="Support this project via Ko-Fi"><img height='24' style='border:0px;height:24px;' src='https://img.shields.io/badge/-Buy%20Me%20a%20Coffee-red?logo=ko-fi&logoColor=white' border='0' alt='Buy Me a Coffee at ko-fi.com' /></a>&thinsp;<a href="https://www.paypal.com/cgi-bin/webscr?cmd=_s-xclick&hosted_button_id=NTZF5G7LRXDRC" title="Support this project via PayPal"><img src="https://img.shields.io/badge/-Donate%20on%20PayPal-yellow.svg?logo=paypal&logoColor=white" alt="Donations" height='24' style='border:0px;height:24px;'></a>
</div>

## 📜 License

This project is licensed under the terms of the [Mozilla Public License v2.0](https://www.mozilla.org/en-US/MPL/2.0/).<br/>
For more details, please check:

+ The [LICENSE](./LICENSE) file.
+ [Mozilla's MPL-2.0 FAQ](https://www.mozilla.org/en-US/MPL/2.0/FAQ/)

Older versions (v3.x and earlier) of bit7z were released under the [GNU General Public License v2](https://www.gnu.org/licenses/old-licenses/gpl-2.0.en.html).

<br/>
<div align="center">
Copyright &copy; 2014 - 2023 Riccardo Ostani (<a href="https://github.com/rikyoz">@rikyoz</a>)
</div>
<br/>
