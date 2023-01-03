<h1 align="center">
bit7z
</h1>
<h3 align="center">A C++ static library offering a clean and simple interface to the 7-zip shared libraries</h3>
<p align="center">
  <a href="#dart-supported-features" title="List of Features Supported by the Library">Supported Features</a> •
  <a href="#fire-getting-started-library-usage" title="Basic Source Code Examples">Getting Started</a> •
  <a href="#file_cabinet-download" title="Download Pre-compiled Packages">Download</a> •
  <a href="#toolbox-requirements" title="Usage Requirements">Requirements</a> •
  <a href="#gear-building-bit7z" title="Building the Library">Building</a> •
  <a href="#coffee-donate" title="Support the Project">Donate</a> •
  <a href="#page_with_curl-license" title="Project License">License</a>
</p>
<p align="center">
  <a href="https://github.com/rikyoz/bit7z/releases" title="Latest Stable GitHub Release"><img src="https://img.shields.io/github/release/rikyoz/bit7z/all.svg?style=flat&logo=github&logoColor=white&colorB=blue&label=" alt="GitHub release"></a>&emsp14;<img src="https://img.shields.io/badge/-C++14/17-3F63B3.svg?style=flat&logo=C%2B%2B&logoColor=white" alt="C++14/17" title="C++ Standards Used: C++14/17">&emsp14;<img src="https://img.shields.io/badge/-Windows-6E46A2.svg?style=flat&logo=windows-11&logoColor=white" alt="Windows" title="Supported Platform: Windows">&emsp14;<img src="https://img.shields.io/badge/-Linux-9C2A91.svg?style=flat&logo=linux&logoColor=white" alt="Linux" title="Supported Platform: Linux">&emsp14;<img src="https://img.shields.io/badge/-macOS-red.svg?style=flat&logo=apple&logoColor=white" alt="macOS" title="Supported Platform: macOS">&emsp14;<img src="https://img.shields.io/badge/-x86%20&middot;%20x64-orange.svg?style=flat&logo=data:image/svg+xml;base64,PHN2ZyB4bWxucz0iaHR0cDovL3d3dy53My5vcmcvMjAwMC9zdmciIHhtbDpzcGFjZT0icHJlc2VydmUiIHZpZXdCb3g9IjAgMCA5NDIgOTQyIj48cGF0aCBmaWxsPSIjZmZmIiBkPSJNNTc5LjEgODk0YTQ4IDQ4IDAgMCAwIDk2IDB2LTc3LjVoLTk1LjlWODk0aC0uMXpNNTc5LjEgNDh2NzcuNUg2NzVWNDhhNDggNDggMCAwIDAtOTUuOSAwek00MjMgNDh2NzcuNWg5NlY0OGE0OCA0OCAwIDAgMC05NiAwek00MjMgODk0YTQ4IDQ4IDAgMCAwIDk2IDB2LTc3LjVoLTk2Vjg5NHpNMjY3IDQ4djc3LjVoOTUuOVY0OGE0OCA0OCAwIDAgMC05NS45IDB6TTI2NyA4OTRhNDggNDggMCAwIDAgOTYgMHYtNzcuNWgtOTZWODk0ek0wIDYyN2E0OCA0OCAwIDAgMCA0OCA0OGg3Ny41di05NS45SDQ4QTQ4IDQ4IDAgMCAwIDAgNjI3ek04OTQgNTc5LjFoLTc3LjVWNjc1SDg5NGE0OCA0OCAwIDAgMCAwLTk1Ljl6TTAgNDcxYTQ4IDQ4IDAgMCAwIDQ4IDQ4aDc3LjV2LTk2SDQ4YTQ4IDQ4IDAgMCAwLTQ4IDQ4ek04OTQgNDIzaC03Ny41djk2SDg5NGE0OCA0OCAwIDAgMCAwLTk2ek0wIDMxNWE0OCA0OCAwIDAgMCA0OCA0OGg3Ny41di05Nkg0OGE0OCA0OCAwIDAgMC00OCA0OHpNODk0IDI2N2gtNzcuNXY5NS45SDg5NGE0OCA0OCAwIDAgMCAwLTk1Ljl6TTE3MS42IDcyMC40YTUwIDUwIDAgMCAwIDUwIDUwaDQ5OC44YTUwIDUwIDAgMCAwIDUwLTUwVjIyMS42YTUwIDUwIDAgMCAwLTUwLTUwSDIyMS42YTUwIDUwIDAgMCAwLTUwIDUwdjQ5OC44eiIvPjwvc3ZnPg==" alt="x86, x64" title="Supported CPU Architectures: x86, x64">&emsp14;<a href="#donations" title="Donate"><img src="https://img.shields.io/badge/-donate-yellow.svg?style=flat&logo=paypal&logoColor=white" alt="donate"></a>&emsp14;<a href="https://github.com/rikyoz/bit7z/wiki" title="Project Documentation"><img src="https://img.shields.io/badge/-docs-green.svg?style=flat&logo=data:image/svg+xml;base64,PHN2ZyB4bWxucz0iaHR0cDovL3d3dy53My5vcmcvMjAwMC9zdmciIGRhdGEtbmFtZT0iTGF5ZXIgMSIgdmlld0JveD0iMCAwIDEwNS4zIDEyMi45Ij48cGF0aCBmaWxsLXJ1bGU9ImV2ZW5vZGQiIGZpbGw9IiNmZmYiIGQ9Ik0xNy41IDBIMTAydjk0LjJjLS4xIDIuNy0zLjUgMi43LTcuMiAyLjZIMTYuM2E5LjIgOS4yIDAgMCAwIDAgMTguNEg5OHYtOS44aDcuMlYxMThhNC4yIDQuMiAwIDAgMS00LjEgNC4xSDE2LjZDNy41IDEyNS41IDAgMTE4IDAgMTA4LjhWMTcuNUExNy42IDE3LjYgMCAwIDEgMTcuNSAwWm0tMS4zIDEwOGg3NS4yYTEuNCAxLjQgMCAwIDEgMS40IDEuM3YuOGExLjQgMS40IDAgMCAxLTEuNCAxLjRIMTYuMmExLjQgMS40IDAgMCAxLTEuMy0xLjR2LS44YTEuNCAxLjQgMCAwIDEgMS4zLTEuNFptMC03LjJoNzUuMmExLjQgMS40IDAgMCAxIDEuNCAxLjR2LjhhMS40IDEuNCAwIDAgMS0xLjQgMS40SDE2LjJBMS40IDEuNCAwIDAgMSAxNSAxMDN2LS44YTEuNCAxLjQgMCAwIDEgMS4zLTEuNFoiLz48L3N2Zz4=" alt="docs"></a>&emsp14;<a href="https://ci.appveyor.com/project/rikyoz/bit7z" title="AppVeyor CI Build Status"><img src="https://img.shields.io/appveyor/ci/rikyoz/bit7z.svg?style=flat&logo=appveyor&logoColor=white&label=" alt="Build status"></a>
  <br>
  <img src="https://img.shields.io/badge/MSVC%202015+-flag.svg?color=555555&style=flat&logo=visual%20studio&logoColor=white" alt="MSVC 2015+" title="Supported Windows Compiler: MSVC 2015 or later">&emsp14;<img src="https://img.shields.io/badge/MinGW%206.4+%20-flag.svg?color=555555&style=flat&logo=windows&logoColor=white" alt="MinGW 6.4+" title="Supported Windows Compiler: MinGW 6.4 or later">&emsp14;<img src="https://img.shields.io/badge/GCC%204.9+-flag.svg?color=555555&style=flat&logo=gnu&logoColor=white" alt="GCC 4.9+" title="Supported Unix Compiler: GCC 4.9 or later">&emsp14;<img src="https://img.shields.io/badge/Clang%203.5+-flag.svg?color=555555&style=flat&logo=llvm&logoColor=white" alt="Clang 3.5+" title="Supported Unix Compiler: Clang 3.5 or later">&emsp14;<a href="https://github.com/rikyoz/bit7z/blob/master/LICENSE" title="Project License: MPLv2"><img src="https://img.shields.io/badge/-MPL--2.0-lightgrey.svg?style=flat&logo=data:image/svg+xml;base64,PHN2ZyB4bWxucz0iaHR0cDovL3d3dy53My5vcmcvMjAwMC9zdmciIGZpbGwtcnVsZT0iZXZlbm9kZCIgY2xpcC1ydWxlPSJldmVub2RkIiBpbWFnZS1yZW5kZXJpbmc9Im9wdGltaXplUXVhbGl0eSIgc2hhcGUtcmVuZGVyaW5nPSJnZW9tZXRyaWNQcmVjaXNpb24iIHRleHQtcmVuZGVyaW5nPSJnZW9tZXRyaWNQcmVjaXNpb24iIHZpZXdCb3g9IjAgMCA1MTIgNDMwLjMiPjxwYXRoIGZpbGw9IiNmZmYiIGQ9Ik0zNC43IDBoNDQyLjZDNDk2LjQgMCA1MTIgMTUuNiA1MTIgMzQuN1YzMzVjMCAxOS0xNS42IDM0LjYtMzQuNyAzNC42aC02OS44Yy0zLjQtNy4yLTctMTQuNS0xMC44LTIxLjdoODIuNWM1LjMgMCA5LjUtNC4yIDkuNS05LjZWOTdoLjJIMjQuN3YyNDEuNGMwIDUuNCA0LjIgOS42IDkuNiA5LjZoODEuMWwtOS4zIDE4LTEuNyAzLjdIMzQuN0MxNS42IDM2OS43IDAgMzU0LjEgMCAzMzUuMVYzNC43QzAgMTUuNiAxNS42IDAgMzQuNyAwem0yMjguOSAxNTFjNSAxLjcgOS4zIDUuMSAxMy4zIDguNmwuMi4yYzkuMSA3LjcgMTAuMyA5IDIyLjQgOS42IDQuNi4yIDkuNS40IDE0IDEuOCA4LjIgMi43IDEzLjIgMTAgMTUuNyAxNy44IDIuMyA3LjIgMSAxNC40IDMuNSAyMS4zIDIuMyA2LjggNy40IDEyIDExIDE4IDMgNC43IDQuOCA5IDUuNyAxM2EyNSAyNSAwIDAgMS03LjYgMjMuOWMtMi4xIDIuMS04IDYuNi05IDguOGEyNi4xIDI2LjEgMCAwIDAtMS43IDExYy0uMSA1LS4yIDEwLTMuNSAxNi42di4xYTMyLjIgMzIuMiAwIDAgMS0xOS40IDE2LjVjLTYuNCAyLTE0LjItLjYtMTguNSAxLjNoLS4yYy04LjcgMy43LTE1LjIgMTIuNy0yMy45IDE1LjVhMzEuNCAzMS40IDAgMCAxLTkuNiAxLjVjLTMuMiAwLTYuMy0uNS05LjUtMS41aC0uMmMtNC4zLTEuNC04LTQuNS0xMi4xLTcuOC0zLjctMy03LjYtNi0xMS44LTcuOC0yLS45LTQuNy0uNi03LjYtLjUtMy40LjItNyAuNS0xMS0uOGgtLjJjLTQuMy0xLjQtOC0zLjUtMTEuMy02LjNhMzMgMzMgMCAwIDEtOC0xMC4zYy0zLjItNi41LTMuMy0xMS43LTMuNC0xNi42LS4yLTkuMi0uMy0xMC43LTYuNy0xNi4yLTguNy03LjQtMTQuNS0xNS4yLTExLjgtMjcuNSAxLjQtNiA1LTEyLjUgMTAuOC0yMC4yYTM3IDM3IDAgMCAwIDYtMTAuOGMyLjMtNi41IDEuNC0xMy40IDMuMS0yMCAyLTcuNyA3LTE1LjQgMTQuNS0xOC41IDUtMiAxMC40LTIuMiAxNS41LTIuNCAxMS44LS40IDEzLjctMiAyMi41LTkuN2E0MSA0MSAwIDAgMSAxMy41LTguNyAyNy4xIDI3LjEgMCAwIDEgMTUuMyAwem04Ny4yIDI1Ny4yLTIzLjQtNC4yLTExLjcgMjEtLjMuNGMtNSA2LTkuNSA2LTEzLjMgMi43LTQuMi0zLjQtNy0xMC43LTguOC0xNS41bC0uNy0xLjgtMjQuOC00Ni44YTIuOSAyLjkgMCAwIDEgMS4yLTRsLjQtLjFjMi42LS45IDUuNC0yLjcgOC42LTUgMy4yLTIuNCA2LjYtNS40IDEwLjEtOC42YTMgMyAwIDAgMSAyLjMtLjdjNi43LjEgMTMuMi0uNCAxOS0yLjQgNS44LTEuOSAxMS01LjMgMTUuNi0xMSAxLTEuNCAyLjktMS42IDQuMS0uNmwuOCAxIDI4LjcgNTUuM2MyLjMgNSA1LjQgMTAuOCAyLjYgMTYuMy0xLjUgMy00LjYgNC42LTkuNyA0LjJsLS43LS4yek0xODQuNyA0MDRsLTIzLjQgNC4yaC0uOGMtNS4xLjQtOC4yLTEuMS05LjctNC4yLTEuMi0yLTEuMi00LjgtLjYtOCAuNi0zLjMgMS44LTUuMSAzLjItOC4ybDI4LjctNTUuNC44LTFjMS4zLTEgMy4xLS43IDQuMS42IDQuNCA1LjYgOS43IDkgMTUuNSAxMSA2IDIuMSAxMi40IDIuNiAxOS4xIDIuNC44IDAgMS41LjIgMi4yLjggMy41IDMuMSA3IDYuMSAxMC4yIDguNiAzLjEgMi4zIDYgNCA4LjYgNWguNGEzIDMgMCAwIDEgMS4yIDRsLTI0LjggNDYuOS0uNyAxLjdBMzguNyAzOC43IDAgMCAxIDIxMCA0MjhjLTMuOCAzLTguMyAzLjMtMTMuMy0yLjcgMC0uMi0uMi0uMy0uMy0uNGwtMTEuNi0yMXptMTAwLTIwMy4yQTUwLjggNTAuOCAwIDAgMCAyNTYgMTkyIDUxIDUxIDAgMCAwIDIwNSAyNDNhNTEgNTEgMCAwIDAgNTEuMSA1MS4yIDUxIDUxIDAgMCAwIDUxLjItNTEuMiA1MS43IDUxLjcgMCAwIDAtNy4yLTI2IDUxLjQgNTEuNCAwIDAgMC0xNS4zLTE2LjN6TTQ0NC4zIDM5YTE3LjEgMTcuMSAwIDEgMSAwIDM0LjIgMTcuMSAxNy4xIDAgMCAxIDAtMzQuMnptLTExNiAwYTE3LjEgMTcuMSAwIDEgMSAwIDM0LjIgMTcuMSAxNy4xIDAgMCAxIDAtMzQuMnptNTggMGExNy4xIDE3LjEgMCAxIDEgMCAzNC4yIDE3LjEgMTcuMSAwIDAgMSAwLTM0LjJ6Ii8+PC9zdmc+" alt="License"></a>
</p>

## :zap: Introduction

**bit7z** is a _cross-platform_ C++ static library that allows the _compression/extraction of archive files_ through a _clean_ and _simple_ wrapper interface to the dynamic libraries from the [7-zip](https://www.7-zip.org/ "7-zip Project Homepage") project.<br/>
It supports compression and extraction to and from the filesystem or the memory, reading archives metadata, updating existing ones, creating multi-volume archives, operation progress callbacks, and many other functionalities.

## :dart: Supported Features

+ **Compression** using the following archive formats: **7z**, XZ, **BZIP2**, **GZIP**, TAR, **ZIP**, and WIM.
+ **Extraction** of many archive formats: **7z**, AR, ARJ, **BZIP2**, CAB, CHM, CPIO, CramFS, DEB, DMG, EXT, FAT, GPT, **GZIP**, HFS, HXS, IHEX, ISO, LZH, LZMA, MBR, MSI, NSIS, NTFS, QCOW2, **RAR**, **RAR5**, RPM, SquashFS, TAR, UDF, UEFI, VDI, VHD, VMDK, WIM, XAR, XZ, Z, and **ZIP**.
+ **Reading metadata** of archives and their content.
+ **Testing** archives for errors.
+ **Updating** existing file archives with new files.
+ **Renaming**, **updating**, or **deleting** old items in existing file archives.
+ **Compression and extraction _to and from_ memory** and **C++ standard streams**.
+ Compression using **custom path aliases** for the items in the output archives.
+ **Selective extraction** of only specified files/folders **using wildcards** and **regexes**.
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
For example, 7z.dll should support all these features, 7za.dll should work only with the 7z file format, and 7zxa.dll can only extract 7z files. For more information about the 7-zip DLLs, please see this [wiki page](https://github.com/rikyoz/bit7z/wiki/7z-DLLs).

In the end, some other features (e.g., _automatic format detection_ and _selective extraction using regexes_) are disabled by default, and macro definitions must be used during compilation to have them available ([wiki](https://github.com/rikyoz/bit7z/wiki/Building-the-library)).

## :fire: Getting Started (Library Usage)

Below are a few examples that show how to use some of the main features of bit7z.

### :open_file_folder: Extracting files from an archive

```cpp
#include <bit7z/bitfileextractor.hpp>

try { //bit7z classes can throw BitException objects
    using namespace bit7z;

    Bit7zLibrary lib{ "7za.dll" };
    BitFileExtractor extractor{ lib, BitFormat::SevenZip };

    //extracting a simple archive
    extractor.extract( "path/to/archive.7z", "out/dir/" );

    //extracting a specific file
    extractor.extractMatching( "path/to/archive.7z", "file.pdf", "out/dir/" );

    //extracting the first file of an archive to a buffer
    std::vector< byte_t > buffer;
    extractor.extract( "path/to/archive.7z", buffer );

    //extracting an encrypted archive
    extractor.setPassword( "password" );
    extractor.extract( "path/to/another/archive.7z", "out/dir/" );
} catch ( const bit7z::BitException& ex ) { /* do something with ex.what()...*/ }
```

### :briefcase: Compressing files into an archive

```cpp
#include <bit7z/bitfilecompressor.hpp>

try { //bit7z classes can throw BitException objects
    using namespace bit7z;

    Bit7zLibrary lib{ "7z.dll" };
    BitFileCompressor compressor{ lib, BitFormat::Zip };

    std::vector< std::string > files = { "path/to/file1.jpg", "path/to/file2.pdf" };

    //creating a simple zip archive
    compressor.compress( files, "output_archive.zip" );

    //creating a zip archive with a custom directory structure
    std::map< std::string, std::string > files_map = {
        { "path/to/file1.jpg", "alias/path/file1.jpg" },
        { "path/to/file2.pdf", "alias/path/file2.pdf" }
    };
    compressor.compress( files_map, "output_archive2.zip" );

    //compressing a directory
    compressor.compressDirectory( "dir/path/", "dir_archive.zip" );

    //creating an encrypted zip archive of two files
    compressor.setPassword( "password" );
    compressor.compressFiles( files, "protected_archive.zip" );

    //updating an existing zip archive
    compressor.setUpdateMode( UpdateMode::Append );
    compressor.compressFiles( files, "existing_archive.zip" );

    //compressing a single file into a buffer
    std::vector< byte_t > buffer;
    BitFileCompressor compressor2{ lib, BitFormat::BZip2 };
    compressor2.compressFile( files[0], buffer );
} catch ( const bit7z::BitException& ex ) { /* do something with ex.what()...*/ }
```

### :bookmark_tabs: Reading archive metadata

```cpp
#include <bit7z/bitarchivereader.hpp>

try { //bit7z classes can throw BitException objects
    using namespace bit7z;

    Bit7zLibrary lib{ "7za.dll" };
    BitArchiveReader arc{ lib, "archive.7z", BitFormat::SevenZip };

    //printing archive metadata
    cout << "Archive properties" << endl;
    cout << " Items count: "   << arc.itemsCount() << endl;
    cout << " Folders count: " << arc.foldersCount() << endl;
    cout << " Files count: "   << arc.filesCount() << endl;
    cout << " Size: "          << arc.size() << endl;
    cout << " Packed size: "   << arc.packSize() << endl;
    cout << endl;

    //printing archive items metadata
    cout << "Archive items";
    auto arc_items = arc.items();
    for ( auto& item : arc_items ) {
        cout << endl;
        cout << " Item index: "   << item.index() << endl;
        cout << "  Name: "        << item.name() << endl;
        cout << "  Extension: "   << item.extension() << endl;
        cout << "  Path: "        << item.path() << endl;
        cout << "  IsDir: "       << item.isDir() << endl;
        cout << "  Size: "        << item.size() << endl;
        cout << "  Packed size: " << item.packSize() << endl;
    }
} catch ( const bit7z::BitException& ex ) { /* do something with ex.what()...*/ }
```

A complete _**API reference**_ is available in the [wiki](https://github.com/rikyoz/bit7z/wiki/) section.

## :rocket: Upgrading from bit7z v3 to v4

The newest bit7z v4 introduced some major breaking changes to the API. In particular:

+ By default, the project now follows the [UTF-8 Everywhere Manifesto](http://utf8everywhere.org/):
  + The default string type is now `std::string` (instead of `std::wstring`) so that the library can be used in cross-platform projects more easily.
  + Input `std::string`s will be considered as UTF-8 encoded.
  + You can still achieve the old behavior by using the `-DBIT7Z_USE_NATIVE_STRING` CMake option.
+ The old `BitExtractor` class is now called `BitFileExtractor`.
  + Now `BitExtractor` is just the name of a template class for all the extraction classes.

## :file_cabinet: Download

<div align="center">
<a href="https://github.com/rikyoz/bit7z/releases/latest">
<img alt="Github All Releases" src="https://img.shields.io/github/v/release/rikyoz/bit7z?label=Latest%20Release&logo=github&style=social" height='36' style='border:0;height:36px;'/></a>
<br/>
<a href="https://github.com/rikyoz/bit7z/releases/latest">
<img alt="Github All Releases" src="https://img.shields.io/github/downloads/rikyoz/bit7z/total.svg?style=popout&label=Total%20Downloads&logo=icloud&logoColor=white"/></a>
</div>

Each released package contains a _precompiled version_ of bit7z (both in _debug_ and _release_ mode) and the _public API headers_ needed to use the library in your program; packages are available for both _x86_ and _x64_ architectures.

You can also clone/download this repository and build the library by yourself (please, read the [wiki](https://github.com/rikyoz/bit7z/wiki/Building-the-library)).

## :toolbox: Requirements

+ **Operating System:** Windows, Linux, macOS<strong>[^1]</strong>.
+ **Architecture:** x86, x86_64.
+ **Compiler:** MSVC 2015 or later<strong>[^2]</strong>, MinGW v6.4 or later, GCC v4.9 or later, Clang 3.5 or later.
+ **Shared Library:** a 7-zip `.dll` library on Windows, a 7-zip/p7zip `.so` library on Unix<strong>[^3]</strong>.

[^1]: On Windows, you should link your program _also_ with _oleaut32_ (e.g., `-lbit7z -loleaut32`).<br/> On Linux and macOS, you should link your program _also_ with _dl_ (e.g., `-lbit7z -ldl`).<br/> If you are using the library via CMake, these dependencies will be linked automatically to your project.

[^2]: MSVC 2010 was supported until v2.x, MSVC 2012/2013 until v3.x.

[^3]: bit7z doesn't ship with the 7-zip shared libraries. You can build them from the source code available at [7-zip.org](http://www.7-zip.org/).

## :gear: Building bit7z

```bash
cd <bit7z folder>
mkdir build && cd build
cmake ../ -DCMAKE_BUILD_TYPE=Release
cmake --build . -j --config Release
```

A more detailed guide on how to build this library is available [here](https://github.com/rikyoz/bit7z/wiki/Building-the-library).

## :coffee: Donate

If you have found this project helpful, please consider supporting me with a small donation so that I can keep improving it!
Thank you! :) :pray:

<div align="center">
<a href='https://github.com/sponsors/rikyoz' target='_blank' title="Support the Maintainer via GitHub Sponsors"><img height='24' style='border:0px;height:24px;' src='https://img.shields.io/badge/-Sponsor%20the%20Maintainer-fafbfc?logo=GitHub%20Sponsors' border='0' alt='Sponsor me on GitHub' /></a> <a href='https://ko-fi.com/G2G1LS1P' target='_blank' title="Support this project via Ko-Fi"><img height='24' style='border:0px;height:24px;' src='https://img.shields.io/badge/-Buy%20Me%20a%20Coffee-red?logo=ko-fi&logoColor=white' border='0' alt='Buy Me a Coffee at ko-fi.com' /></a> <a href="https://www.paypal.com/cgi-bin/webscr?cmd=_s-xclick&hosted_button_id=NTZF5G7LRXDRC" title="Support this project via PayPal"><img src="https://img.shields.io/badge/-Donate%20on%20PayPal-yellow.svg?logo=data:image/svg+xml;base64,PHN2ZyB4bWxucz0iaHR0cDovL3d3dy53My5vcmcvMjAwMC9zdmciIHZpZXdCb3g9IjAgMCAyNCAyNCI+PHBhdGggZmlsbD0iI0ZGRiIgZD0iTTcuMDc2IDIxLjMzN0gyLjQ3YS42NDEuNjQxIDAgMCAxLS42MzMtLjc0TDQuOTQ0LjkwMUM1LjAyNi4zODIgNS40NzQgMCA1Ljk5OCAwaDcuNDZjMi41NyAwIDQuNTc4LjU0MyA1LjY5IDEuODEgMS4wMSAxLjE1IDEuMzA0IDIuNDIgMS4wMTIgNC4yODctLjAyMy4xNDMtLjA0Ny4yODgtLjA3Ny40MzctLjk4MyA1LjA1LTQuMzQ5IDYuNzk3LTguNjQ3IDYuNzk3aC0yLjE5Yy0uNTI0IDAtLjk2OC4zODItMS4wNS45bC0xLjEyIDcuMTA2em0xNC4xNDYtMTQuNDJhMy4zNSAzLjM1IDAgMCAwLS42MDctLjU0MWMtLjAxMy4wNzYtLjAyNi4xNzUtLjA0MS4yNTQtLjkzIDQuNzc4LTQuMDA1IDcuMjAxLTkuMTM4IDcuMjAxaC0yLjE5YS41NjMuNTYzIDAgMCAwLS41NTYuNDc5bC0xLjE4NyA3LjUyN2gtLjUwNmwtLjI0IDEuNTE2YS41Ni41NiAwIDAgMCAuNTU0LjY0N2gzLjg4MmMuNDYgMCAuODUtLjMzNC45MjItLjc4OC4wNi0uMjYuNzYtNC44NTIuODE2LTUuMDlhLjkzMi45MzIgMCAwIDEgLjkyMy0uNzg4aC41OGMzLjc2IDAgNi43MDUtMS41MjggNy41NjUtNS45NDYuMzYtMS44NDcuMTc0LTMuMzg4LS43NzctNC40NzF6Ii8+PC9zdmc+&logoColor=white" alt="Donations" height='24' style='border:0px;height:24px;'></a>
</div>

## :page_with_curl: License

This project is licensed under the terms of the [Mozilla Public License v2.0](https://www.mozilla.org/en-US/MPL/2.0/).<br/>
For more details, please check:

+ The [LICENSE](./LICENSE) file.
+ [Mozilla's MPL-2.0 FAQ](https://www.mozilla.org/en-US/MPL/2.0/FAQ/)

Older versions (v3.x and earlier) of bit7z were released under the [GNU General Public License v2](https://www.gnu.org/licenses/old-licenses/gpl-2.0.en.html).

<br/>
<div align="center">
Copyright &copy; 2014 - 2022 Riccardo Ostani (<a href="https://github.com/rikyoz">@rikyoz</a>)
</div>
<br/>
