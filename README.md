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
  <a href="https://github.com/rikyoz/bit7z/releases" title="Latest Stable GitHub Release"><img src="https://img.shields.io/github/release/rikyoz/bit7z/all.svg?style=flat&logo=github&logoColor=white&colorB=blue&label=" alt="GitHub release"></a>&emsp14;<img src="https://img.shields.io/badge/-C++14/17-3F63B3.svg?style=flat&logo=C%2B%2B&logoColor=white" alt="C++14/17" title="C++ Standards Used: C++14/17">&emsp14;<img src="https://img.shields.io/badge/-Windows-6E46A2.svg?style=flat&logo=windows&logoColor=white" alt="Windows" title="Supported Platform: Windows">&emsp14;<img src="https://img.shields.io/badge/-Linux-9C2A91.svg?style=flat&logo=linux&logoColor=white" alt="Linux" title="Supported Platform: Linux">&emsp14;<img src="https://img.shields.io/badge/-macOS-red.svg?style=flat&logo=apple&logoColor=white" alt="macOS" title="Supported Platform: macOS">&emsp14;<img src="https://img.shields.io/badge/-x86%20&middot;%20x64-orange.svg?style=flat&logo=data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAAEAAAABACAQAAAAAYLlVAAAC1UlEQVR42u3WA9AjWRSA0bu2bdu2bdssrm3btm3btm3bmX+Ms7rLTiW9NUmvcsL7XuELu6Ojoz5DWcc5nvKp2kBdPvesi21m1Pgr7OxjrfWtgw0VZZjKM9rjXfNHM+bWWzutGo2YSH/tNm+jgNe1XzdDR322V41Tox5D6K4qY0WRtVRnjyhysercH0VeVJ13o8hXqvNNFOlSna4oUlOd2r8moBPwoQfd6THfoLweauqp6aJ8wInmMmjujWAFtwMeNJup5cXsVnWYDyDtajQjmMp7QOoypxGMbMtyAe+Ztf5/JTaJAkM6mjRXrj0KpE9zdZIyAV8bLX5lBIPlszXAVlGXMwAr5fwskL4wdPzAfGUC5o9kJy+o+dCVloiwJNg2907wimddZrqcB9GtNQF3RXI+kI5yCcgADwF6yvfLNa0JWD7n5dWXAa4lbZwrR7UioKdhc76vdEB+KxzbioAncxpGr9IBM+XKDa0IuCanaWkS8BzguEhqrQg4P6e5mgasbV+7WCySvWlFwIU5zdYooMhytCbghpzGLh9gAodCWjFXXwDSV4aJH5inWcBLkbzTOMBa9rWvk92jH5BWqBvwjSHKBfQ3as4HlvoSFq2b+zcB6bXIj6pZABvnPKzPgPSJlxV/hkUH5v7SUPiv2LN5wKuRjO82wDdON6xFSwW8XvhdcGYkrzUPYJf4lcktZh4jxg8sViqA9SKZxDo2NH0km1ImgE2jDjuBLXK6FPX1N1fUYQnKBnCeGeN3jGdPfUC+P27TyO7GjN8xoUMpHZCecKZ97etE9+hD6vKQOz1jgMa6u90J+VO9V//OaXnzgE5Al+p0iyLfqM63UeRV1Xk/ilylOo9Gkc1U55AoMrz+qjJJ1OMQ1bgq6jOYr1Rh9EgFZtd+q0QjVtFeW0UzFvGJ9uhhrSjDSE7UX6tdaMIoz0R2cbvXfKE2UJevvOEe+5kuOjr+qb4H0/HV/SQ0YjEAAAAASUVORK5CYII=" alt="x86, x64" title="Supported CPU Architectures: x86, x64">&emsp14;<a href="#donations" title="Donate"><img src="https://img.shields.io/badge/-donate-yellow.svg?style=flat&logo=data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAAEAAAABBCAMAAABW61JJAAAAM1BMVEUAAAD///////////////////////////////////////////////////////////////+3leKCAAAAEHRSTlMAv0CA7xBgrzDfn3BQIM+PWN4WLwAAAfhJREFUeF69llmS3CAQRLMWNi0t7n9ax/RI7lCOQMh2+H0DovQqo8A/RpNcksw0455Yu8i8ostabxFFB6sDSOcWqY7gC1p4HSPimlxHyU8dMAWXTJV4WkSpf3eFUIkib7xekC96aqE1M3ayeWXmxSOIjfd/CFMlNjjbhFCV3R4TJLaJemLqx0RgbFOpgr4igfKiud9scnVAaVfpuD2AP+O0gHjxL1JSRX/JbrrMoPSrKUl6EzRFpItSkm4qqICdDuAVLwAdRTXtWqRV48YFELpvkFaSllNMt4s0R5JlzRuEWCqjR3JSq0/2rKqa1J8kQNl2fcArAEK2tY7j+WMlkKbR/dl3m5SkEUoGQmFZ8DqIBCAcwSqUpHt8BpALBWZ8Jk0BwOIfm49mkk8rgJDqQQmUpC4pvpebn2xQkhq4mH4vs3K2yUn68JI3ZrZoOCKR2CZaSXLFmXUxYZvgmUSXy/rFbGYilzb7MynInc3eTIpdK76tIDhJa7uvPMX71503+qok06HXXQLKyeZkZqrjD9z56Kui6NGZScths0tnJm00GvqwMwFktzkGJ8mAw2af9kxSfmT14SQF2G5zFOGxnHabf3aAHX2VMcpGScy7zWGCycE7KkZz+yE8dsdZ9YtlopkxzsY2n8KPrMc42XzMRIl6TJjkNxH/kV+EeH5bYbU1DgAAAABJRU5ErkJggg==&logoColor=white" alt="donate"></a>&emsp14;<a href="https://github.com/rikyoz/bit7z/wiki" title="Project Documentation"><img src="https://img.shields.io/badge/-docs-green.svg?style=flat&logo=data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAAEAAAABACAQAAAAAYLlVAAABCklEQVR42u3OAUQEARCF4SkKTgASgCIhCBRAQXHVSeEEgY6gAIggQiIACAISyAEkqigRiAKQAItEOlj1B1HU7nB2VvS+B4D3W/sYZZNzHnmlxQMHzNJhZWGCS346pdfi0c0u7/zulkr8fZM8WxaLffI902VxqOIbszhc41u0KAwAvoZFoQb4qhaFBr6UPotCHd+RxWEYzwv9Fok78qTMWyymeCNLizmLx2pGwhWDVg7GueG7lBOm6bQyMcQS62ywwiQ9JiJ/ARVGqLPDBctWLPY4zFyTY86454kva1YsEnwoIDJghoXPbUcHOKj9zwAFKEABClCAAhSgAAUoQAEKUIACSGhXYgpQgOsD2giqlbnGmc4AAAAASUVORK5CYII=" alt="docs"></a>&emsp14;<a href="https://ci.appveyor.com/project/rikyoz/bit7z" title="AppVeyor CI Build Status"><img src="https://img.shields.io/appveyor/ci/rikyoz/bit7z.svg?style=flat&logo=appveyor&logoColor=white&label=" alt="Build status"></a>
  <br>
  <img src="https://img.shields.io/badge/MSVC%202015+-flag.svg?color=555555&style=flat&logo=visual%20studio&logoColor=white" alt="MSVC 2015+" title="Supported Windows Compiler: MSVC 2015 or later">&emsp14;<img src="https://img.shields.io/badge/MinGW%206.4+%20-flag.svg?color=555555&style=flat&logo=windows&logoColor=white" alt="MinGW 6.4+" title="Supported Windows Compiler: MinGW 6.4 or later">&emsp14;<img src="https://img.shields.io/badge/GCC%204.9+-flag.svg?color=555555&style=flat&logo=gnu&logoColor=white" alt="GCC 4.9+" title="Supported Unix Compiler: GCC 4.9 or later">&emsp14;<img src="https://img.shields.io/badge/Clang%203.5+-flag.svg?color=555555&style=flat&logo=llvm&logoColor=white" alt="Clang 3.5+" title="Supported Unix Compiler: Clang 3.5 or later">&emsp14;<a href="https://github.com/rikyoz/bit7z/blob/master/LICENSE" title="Project License: MPLv2"><img src="https://img.shields.io/badge/-MPL--2.0-lightgrey.svg?style=flat&logo=data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAAEAAAABACAQAAAAAYLlVAAAAvUlEQVR42u3Zt1EEURRE0Ye0gFwI4BWJoC0KGxftExk+RQATAN+nLpo1R0+v6NsJHPOLcO4vKkrPVexEcMFZN8AQ7UXwCBx0ART6VtiN4BCA1AHO+SnVAEg1AFINgFQDINUASDUAUg2AVAMg1QBINQBSDYBUAyDVAMhxAVfUdzkmYJ+7mj1xPQ7gjbWYJTmQbGsB77zy0mjPPQH9UwOKAQYYYIABBhhggAFzCTDAAAMMMGDS+v2a9V8Vzs1PH+dRolvEzoAoAAAAAElFTkSuQmCC" alt="License"></a>
</p>

## :zap: Introduction

**bit7z** is a _cross-platform_ C++ static library that allows the _compression/extraction of archive files_ through a _clean_ and _simple_ wrapper interface to the dynamic libraries from the [7-zip](https://www.7-zip.org/ "7-zip Project Homepage") project.<br/>
It supports compression and extraction to and from the filesystem or the memory, reading archives metadata, updating existing ones, creating multi-volume archives, operation progress callbacks, and many other functionalities.

## :dart: Supported Features

+ **Compression** using the following archive formats: **7z**, XZ, **BZIP2**, **GZIP**, TAR, **ZIP**, and WIM.
+ **Extraction** of many archive formats: **7z**, AR, ARJ, **BZIP2**, CAB, CHM, CPIO, CramFS, DEB, DMG, EXT, FAT, GPT, **GZIP**, HFS, HXS, IHEX, ISO, LZH, LZMA, MBR, MSI, NSIS, NTFS, QCOW2, **RAR**, **RAR5**, RPM, SquashFS, TAR, UDF, UEFI, VDI, VHD, VMDK, WIM, XAR, XZ, Z, and **ZIP**.
+ **Reading metadata** of archives and of their content.
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
+ **Operation callbacks** for obtaining real-time information about the ongoing operations.
+ **Canceling** or **pausing** the current operation.

### Notes

The presence or not of some of the above features depends on the particular shared library used along with bit7z.<br/>
For example, the 7z.dll should support all these features, 7za.dll should work only with the 7z file format, and the 7zxa.dll can only extract 7z files. For more information about the 7-zip DLLs, please see this [wiki page](https://github.com/rikyoz/bit7z/wiki/7z-DLLs).

In the end, some other features (e.g., _automatic format detection_ and _selective extraction using regexes_) are disabled by default, and macro defines must be used during compilation to have them available ([wiki](https://github.com/rikyoz/bit7z/wiki/Building-the-library)).

## :fire: Getting Started (Library Usage)

Below are a few examples that show how to use some of the main features of bit7z.
Note: the following code is for Windows, where bit7z uses `std::wstring` for string parameters; the library uses `std::string` parameters on Unix systems.

### :open_file_folder: Extracting files from an archive

```cpp
#include "bitfileextractor.hpp"

try { //bit7z classes can throw BitException objects
    using namespace bit7z;

    Bit7zLibrary lib{ L"7za.dll" };
    BitFileExtractor extractor{ lib, BitFormat::SevenZip };

    //extracting a simple archive
    extractor.extract( L"path/to/archive.7z", L"out/dir/" );

    //extracting a specific file
    extractor.extractMatching( L"path/to/archive.7z", L"file.pdf", L"out/dir/" );

    //extracting the first file of an archive to a buffer
    std::vector< byte_t > buffer;
    extractor.extract( L"path/to/archive.7z", buffer );

    //extracting an encrypted archive
    extractor.setPassword( L"password" );
    extractor.extract( L"path/to/another/archive.7z", L"out/dir/" );
} catch ( const bit7z::BitException& ex ) { /* do something with ex.what()...*/ }
```

### :briefcase: Compressing files into an archive

```cpp
#include "bitfilecompressor.hpp"

try { //bit7z classes can throw BitException objects
    using namespace bit7z;

    Bit7zLibrary lib{ L"7z.dll" };
    BitFileCompressor compressor{ lib, BitFormat::Zip };

    std::vector< std::wstring > files = { L"path/to/file1.jpg", L"path/to/file2.pdf" };

    //creating a simple zip archive
    compressor.compress( files, L"output_archive.zip" );

    //creating a zip archive with a custom directory structure
    std::map< std::wstring, std::wstring > files_map = {
        { L"path/to/file1.jpg", L"alias/path/file1.jpg" },
        { L"path/to/file2.pdf", L"alias/path/file2.pdf" }
    };
    compressor.compress( files_map, L"output_archive2.zip" );

    //compressing a directory
    compressor.compressDirectory( L"dir/path/", L"dir_archive.zip" );

    //creating an encrypted zip archive of two files
    compressor.setPassword( L"password" );
    compressor.compressFiles( files, L"protected_archive.zip" );

    //updating an existing zip archive
    compressor.setUpdateMode( UpdateMode::Append );
    compressor.compressFiles( files, L"existing_archive.zip" );

    //compressing a single file into a buffer
    std::vector< byte_t > buffer;
    BitFileCompressor compressor2{ lib, BitFormat::BZip2 };
    compressor2.compressFile( files[0], buffer );
} catch ( const bit7z::BitException& ex ) { /* do something with ex.what()...*/ }
```

### :bookmark_tabs: Reading archive metadata

```cpp
#include "bitarchivereader.hpp"

try { //bit7z classes can throw BitException objects
    using namespace bit7z;

    Bit7zLibrary lib{ L"7za.dll" };
    BitArchiveReader arc{ lib, L"archive.7z", BitFormat::SevenZip };

    //printing archive metadata
    wcout << L"Archive properties" << endl;
    wcout << L" Items count: "   << arc.itemsCount() << endl;
    wcout << L" Folders count: " << arc.foldersCount() << endl;
    wcout << L" Files count: "   << arc.filesCount() << endl;
    wcout << L" Size: "          << arc.size() << endl;
    wcout << L" Packed size: "   << arc.packSize() << endl;
    wcout << endl;

    //printing archive items metadata
    wcout << L"Archive items";
    auto arc_items = arc.items();
    for ( auto& item : arc_items ) {
        wcout << endl;
        wcout << L" Item index: "   << item.index() << endl;
        wcout << L"  Name: "        << item.name() << endl;
        wcout << L"  Extension: "   << item.extension() << endl;
        wcout << L"  Path: "        << item.path() << endl;
        wcout << L"  IsDir: "       << item.isDir() << endl;
        wcout << L"  Size: "        << item.size() << endl;
        wcout << L"  Packed size: " << item.packSize() << endl;
    }
} catch ( const bit7z::BitException& ex ) { /* do something with ex.what()...*/ }
```

A complete _**API reference**_ is available in the [wiki](https://github.com/rikyoz/bit7z/wiki/) section.

## :file_cabinet: Download

<div align="center">
<a href="https://github.com/rikyoz/bit7z/releases/latest">
<img alt="Github All Releases" src="https://img.shields.io/github/v/release/rikyoz/bit7z?label=Latest%20Release&logo=github&style=social" height='36' style='border:0;height:36px;'/></a>
<br/>
<a href="https://github.com/rikyoz/bit7z/releases/latest">
<img alt="Github All Releases" src="https://img.shields.io/github/downloads/rikyoz/bit7z/total.svg?style=popout&label=total%20downloads&logo=icloud&logoColor=white"/></a>
</div>

Each released package contains a _precompiled version_ of bit7z (both in _debug_ and _release_ mode) and the _public API headers_ needed to use the library in your program; packages are available for both _x86_ and _x64_ architectures.

You can also clone/download this repository and build the library by yourself (please, see the [wiki](https://github.com/rikyoz/bit7z/wiki/Building-the-library)).

## :toolbox: Requirements

+ **Operating System:** Windows, Linux, macOS<strong><sup>1</sup></strong>.
+ **Architecture:** x86, x86_64.
+ **Compiler:** MSVC 2015 or later<strong><sup>2</sup></strong>, MinGW v6.4 or later, GCC v4.9 or later, Clang 3.5 or later.
+ **Shared Library:** a 7-zip `.dll` library on Windows, a 7-zip/p7zip `.so` library on Unix<strong><sup>3</sup></strong>.

**<sup>1</sup>** On Windows, you should link your program _also_ with *oleaut32* (e.g., `-lbit7z -loleaut32`).<br/>
On Unix systems (e.g., Linux, macOS), you should link your program _also_ with *dl* (e.g., `-lbit7z -ldl`).

**<sup>2</sup>** MSVC 2010 was supported until v2.x, MSVC 2012/2013 until v3.1.x.

**<sup>3</sup>** bit7z doesn't ship with the 7-zip shared libraries. You can build them from the source code available at [7-zip.org](http://www.7-zip.org/).

## :gear: Building bit7z

### Using CMake

```bash
mkdir build && cd build
cmake ../ -DCMAKE_BUILD_TYPE=Release
cmake --build . -j --config Release
```

### Using MSBuild

```bash
msbuild bit7z.vcxproj /p:configuration=release
```

A more detailed guide on how to build this library is available [here](https://github.com/rikyoz/bit7z/wiki/Building-the-library).

## :coffee: Donate

If you have found this project helpful, please consider supporting me with a small donation so that I can keep improving it!
Thank you! :)

<div align="center">
<a href='https://github.com/sponsors/rikyoz' target='_blank' title="Support the maintainer via GitHub Sponsors"><img height='24' style='border:0px;height:24px;' src='https://img.shields.io/badge/-Sponsor%20the%20maintainer-fafbfc?logo=GitHub%20Sponsors' border='0' alt='Sponsor me on GitHub' /></a> <a href='https://ko-fi.com/G2G1LS1P' target='_blank' title="Support this project via Ko-Fi"><img height='24' style='border:0px;height:24px;' src='https://img.shields.io/badge/-Buy%20Me%20a%20Coffee-red?logo=ko-fi&logoColor=white' border='0' alt='Buy Me a Coffee at ko-fi.com' /></a> <a href="https://www.paypal.com/cgi-bin/webscr?cmd=_s-xclick&hosted_button_id=NTZF5G7LRXDRC" title="Support this project via PayPal"><img src="https://img.shields.io/badge/-Donate%20on%20PayPal-yellow.svg?logo=data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAAEAAAABBCAMAAABW61JJAAAAM1BMVEUAAAD///////////////////////////////////////////////////////////////+3leKCAAAAEHRSTlMAv0CA7xBgrzDfn3BQIM+PWN4WLwAAAfhJREFUeF69llmS3CAQRLMWNi0t7n9ax/RI7lCOQMh2+H0DovQqo8A/RpNcksw0455Yu8i8ostabxFFB6sDSOcWqY7gC1p4HSPimlxHyU8dMAWXTJV4WkSpf3eFUIkib7xekC96aqE1M3ayeWXmxSOIjfd/CFMlNjjbhFCV3R4TJLaJemLqx0RgbFOpgr4igfKiud9scnVAaVfpuD2AP+O0gHjxL1JSRX/JbrrMoPSrKUl6EzRFpItSkm4qqICdDuAVLwAdRTXtWqRV48YFELpvkFaSllNMt4s0R5JlzRuEWCqjR3JSq0/2rKqa1J8kQNl2fcArAEK2tY7j+WMlkKbR/dl3m5SkEUoGQmFZ8DqIBCAcwSqUpHt8BpALBWZ8Jk0BwOIfm49mkk8rgJDqQQmUpC4pvpebn2xQkhq4mH4vs3K2yUn68JI3ZrZoOCKR2CZaSXLFmXUxYZvgmUSXy/rFbGYilzb7MynInc3eTIpdK76tIDhJa7uvPMX71503+qok06HXXQLKyeZkZqrjD9z56Kui6NGZScths0tnJm00GvqwMwFktzkGJ8mAw2af9kxSfmT14SQF2G5zFOGxnHabf3aAHX2VMcpGScy7zWGCycE7KkZz+yE8dsdZ9YtlopkxzsY2n8KPrMc42XzMRIl6TJjkNxH/kV+EeH5bYbU1DgAAAABJRU5ErkJggg==&logoColor=white" alt="Donations" height='24' style='border:0px;height:24px;'></a>
</div>

## :page_with_curl: License

This project is licensed under the terms of the [Mozilla Public License v2.0](https://www.mozilla.org/en-US/MPL/2.0/).<br/>
For more details, please check:
- The [LICENSE](./LICENSE) file.
- [Mozilla's MPL-2.0 FAQ](https://www.mozilla.org/en-US/MPL/2.0/FAQ/)

<br/>
<div align="center">
Copyright &copy; 2014 - 2022 Riccardo Ostani (<a href="https://github.com/rikyoz">@rikyoz</a>)
</div>
