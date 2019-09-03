<h1 align="center">
bit7z
</h1>

<h3 align="center">A C++ static library offering a clean and simple interface to the 7-zip DLLs</h3>

<p align="center">
  <a href="https://github.com/rikyoz/bit7z/releases"><img src="https://img.shields.io/github/release/rikyoz/bit7z/all.svg?style=flat-square&logo=github&logoColor=white&colorB=blue&label=" alt="GitHub release"></a>
  <img src="https://img.shields.io/badge/-MSVC%202012%20--%202019-red.svg?style=flat-square&logo=visual-studio-code&logoColor=white" alt="MSVC version">
  <img src="https://img.shields.io/badge/-x86,%20x86__64-orange.svg?style=flat-square&logo=data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAAEAAAABACAQAAAAAYLlVAAAC1UlEQVR42u3WA9AjWRSA0bu2bdu2bdssrm3btm3btm3bmX+Ms7rLTiW9NUmvcsL7XuELu6Ojoz5DWcc5nvKp2kBdPvesi21m1Pgr7OxjrfWtgw0VZZjKM9rjXfNHM+bWWzutGo2YSH/tNm+jgNe1XzdDR322V41Tox5D6K4qY0WRtVRnjyhysercH0VeVJ13o8hXqvNNFOlSna4oUlOd2r8moBPwoQfd6THfoLweauqp6aJ8wInmMmjujWAFtwMeNJup5cXsVnWYDyDtajQjmMp7QOoypxGMbMtyAe+Ztf5/JTaJAkM6mjRXrj0KpE9zdZIyAV8bLX5lBIPlszXAVlGXMwAr5fwskL4wdPzAfGUC5o9kJy+o+dCVloiwJNg2907wimddZrqcB9GtNQF3RXI+kI5yCcgADwF6yvfLNa0JWD7n5dWXAa4lbZwrR7UioKdhc76vdEB+KxzbioAncxpGr9IBM+XKDa0IuCanaWkS8BzguEhqrQg4P6e5mgasbV+7WCySvWlFwIU5zdYooMhytCbghpzGLh9gAodCWjFXXwDSV4aJH5inWcBLkbzTOMBa9rWvk92jH5BWqBvwjSHKBfQ3as4HlvoSFq2b+zcB6bXIj6pZABvnPKzPgPSJlxV/hkUH5v7SUPiv2LN5wKuRjO82wDdON6xFSwW8XvhdcGYkrzUPYJf4lcktZh4jxg8sViqA9SKZxDo2NH0km1ImgE2jDjuBLXK6FPX1N1fUYQnKBnCeGeN3jGdPfUC+P27TyO7GjN8xoUMpHZCecKZ97etE9+hD6vKQOz1jgMa6u90J+VO9V//OaXnzgE5Al+p0iyLfqM63UeRV1Xk/ilylOo9Gkc1U55AoMrz+qjJJ1OMQ1bgq6jOYr1Rh9EgFZtd+q0QjVtFeW0UzFvGJ9uhhrSjDSE7UX6tdaMIoz0R2cbvXfKE2UJevvOEe+5kuOjr+qb4H0/HV/SQ0YjEAAAAASUVORK5CYII=" alt="Architectures">
  <a href="https://github.com/rikyoz/bit7z/wiki"><img src="https://img.shields.io/badge/-docs-yellow.svg?style=flat-square&logo=data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAAEAAAABACAQAAAAAYLlVAAABCklEQVR42u3OAUQEARCF4SkKTgASgCIhCBRAQXHVSeEEgY6gAIggQiIACAISyAEkqigRiAKQAItEOlj1B1HU7nB2VvS+B4D3W/sYZZNzHnmlxQMHzNJhZWGCS346pdfi0c0u7/zulkr8fZM8WxaLffI902VxqOIbszhc41u0KAwAvoZFoQb4qhaFBr6UPotCHd+RxWEYzwv9Fok78qTMWyymeCNLizmLx2pGwhWDVg7GueG7lBOm6bQyMcQS62ywwiQ9JiJ/ARVGqLPDBctWLPY4zFyTY86454kva1YsEnwoIDJghoXPbUcHOKj9zwAFKEABClCAAhSgAAUoQAEKUIACSGhXYgpQgOsD2giqlbnGmc4AAAAASUVORK5CYII="></a>
  <a href="#donations"><img src="https://img.shields.io/badge/-donations-green.svg?style=flat-square&logo=data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAAEAAAABBCAMAAABW61JJAAAAM1BMVEUAAAD///////////////////////////////////////////////////////////////+3leKCAAAAEHRSTlMAv0CA7xBgrzDfn3BQIM+PWN4WLwAAAfhJREFUeF69llmS3CAQRLMWNi0t7n9ax/RI7lCOQMh2+H0DovQqo8A/RpNcksw0455Yu8i8ostabxFFB6sDSOcWqY7gC1p4HSPimlxHyU8dMAWXTJV4WkSpf3eFUIkib7xekC96aqE1M3ayeWXmxSOIjfd/CFMlNjjbhFCV3R4TJLaJemLqx0RgbFOpgr4igfKiud9scnVAaVfpuD2AP+O0gHjxL1JSRX/JbrrMoPSrKUl6EzRFpItSkm4qqICdDuAVLwAdRTXtWqRV48YFELpvkFaSllNMt4s0R5JlzRuEWCqjR3JSq0/2rKqa1J8kQNl2fcArAEK2tY7j+WMlkKbR/dl3m5SkEUoGQmFZ8DqIBCAcwSqUpHt8BpALBWZ8Jk0BwOIfm49mkk8rgJDqQQmUpC4pvpebn2xQkhq4mH4vs3K2yUn68JI3ZrZoOCKR2CZaSXLFmXUxYZvgmUSXy/rFbGYilzb7MynInc3eTIpdK76tIDhJa7uvPMX71503+qok06HXXQLKyeZkZqrjD9z56Kui6NGZScths0tnJm00GvqwMwFktzkGJ8mAw2af9kxSfmT14SQF2G5zFOGxnHabf3aAHX2VMcpGScy7zWGCycE7KkZz+yE8dsdZ9YtlopkxzsY2n8KPrMc42XzMRIl6TJjkNxH/kV+EeH5bYbU1DgAAAABJRU5ErkJggg==&logoColor=white"></a>
  <a href="https://ci.appveyor.com/project/rikyoz/bit7z"><img src="https://img.shields.io/appveyor/ci/rikyoz/bit7z.svg?style=flat-square&logo=appveyor&logoColor=white&label=" alt="Build status"></a>
  <a href="https://github.com/rikyoz/bit7z/blob/master/LICENSE"><img src="https://img.shields.io/badge/-GPL%20v2-lightgrey.svg?style=flat-square&logo=data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAAEAAAABACAQAAAAAYLlVAAAAvUlEQVR42u3Zt1EEURRE0Ye0gFwI4BWJoC0KGxftExk+RQATAN+nLpo1R0+v6NsJHPOLcO4vKkrPVexEcMFZN8AQ7UXwCBx0ART6VtiN4BCA1AHO+SnVAEg1AFINgFQDINUASDUAUg2AVAMg1QBINQBSDYBUAyDVAMhxAVfUdzkmYJ+7mj1xPQ7gjbWYJTmQbGsB77zy0mjPPQH9UwOKAQYYYIABBhhggAFzCTDAAAMMMGDS+v2a9V8Vzs1PH+dRolvEzoAoAAAAAElFTkSuQmCC" alt="License"></a>
</p>
<p align="center">
  <a href="#supported-features">Supported Features</a> •
  <a href="#getting-started-library-usage">Getting Started</a> •
  <a href="#download">Download</a> •
  <a href="#requirements">Requirements</a> •
  <a href="#building-bit7z">Building</a> •
  <a href="#donations">Donations</a> •
  <a href="#license-gpl-v2">License</a>
</p>

## Introduction

**bit7z** is a C++ static library which allows to compress and extract many file archive formats,  all through a clean, simple and entirely object-oriented interface to the dynamic libraries from the 7-zip project (https://www.7-zip.org/).<br/>It supports compression and extraction to and from the filesystem or the memory, reading of archives metadata, updating existing archives, creation of multi-volume archives, operation progress callbacks and many other functionalities.

## Supported Features

+ **Compression** of files and directories using the following archive formats: 7z, XZ, BZIP2, GZIP, TAR, ZIP and WIM.
+ **Extraction** of the following archive formats: 7z, AR, ARJ, BZIP2, CAB, CHM, CPIO, CramFS, DEB, DMG, EXT, FAT, GPT, GZIP, HFS, HXS, IHEX, ISO, LZH, LZMA, MBR, MSI, NSIS, NTFS, QCOW2, RAR, RAR5, RPM, SquashFS, TAR, UDF, UEFI, VDI, VHD, VMDK, WIM, XAR, XZ, Z and ZIP.
+ **Reading metadata** of archives and of their content (from v3.x).
+ **Testing** archives for errors (from v3.x).
+ **Updating** existing file archives (from v3.1.x).
+ **Compression and extraction _to and from_ memory** (from v2.x &mdash; compression to memory is supported only for BZIP2, GZIP, XZ and TAR formats).
+ **Compression and extraction _to and from_ C++ standard streams** (from v3.1.x).
+ Compression using a **custom directory system** in the output archives (from v3.x)
+ **Selective extraction** of only specified files/folders **using wildcards** (from v3.x) and **regexes** (from v3.1.x).
+ Creation of **encrypted archives** (strong AES-256 encryption &mdash; only for 7z and ZIP formats).
+ **Archive header encryption** (only for 7z format).
+ Choice of the **compression level** (from none to ultra, not all supported by every output archive format).
+ Choice of the **compression method** (from v3.1.x &mdash; see the [wiki](https://github.com/rikyoz/bit7z/wiki/Advanced-Usage#compression-methods) for the supported methods).
+ Choice of the compression **dictionary size** (from v3.1.x).
+ **Automatic input archive format detection** (from v3.1.x).
+ **Solid archives** (only for 7z).
+ **Multi-volume archives** (from v2.1.x).
+ **Operation callbacks**, through which it is possible to obtain real time information about the ongoing extraction or compression operation (from v2.1.x).

Please note that the presence or not of some of the above features depends on the particular .dll used along with bit7z.

For example, the 7z.dll should support all these features, while 7za.dll should support only the 7z file format and the 7zxa.dll can only extract 7z files. For more information about the 7-zip DLLs, please see this [wiki page](https://github.com/rikyoz/bit7z/wiki/7z-DLLs).

In the end, some other features (e.g. _automatic format detection_ and _selective extraction using regexes_) are disabled by default and macros defines must be used during compilation to have them available ([wiki](https://github.com/rikyoz/bit7z/wiki/Building-the-library)).

## Getting Started (Library Usage)

Below are a few examples that show how to use some of the main features of bit7z:

### Extracting files from an archive

```cpp
#include "bitextractor.hpp"

int main(){
    Bit7zLibrary lib{ L"7za.dll" };
    BitExtractor extractor{ lib, BitFormat::SevenZip };

    //extracts a simple archive
    extractor.extract( L"path/to/archive.7z", L"output/dir/" );

    //extracts a specific file from the archive
    extractor.extractMatching( L"path/to/archive.7z", L"file.pdf", L"output/dir/" );

    //extracts the first file of an archive to a buffer
    std::vector< byte_t > buffer;
    extractor.extract( L"path/to/archive.7z", buffer );

    //extracts an encrypted archive
    extractor.setPassword( L"password" );
    extractor.extract( L"path/to/another/archive.7z", L"output/dir/" );
    return 0;
}
```

### Compressing files into an archive

```cpp
#include "bitcompressor.hpp"

int main(){
    Bit7zLibrary lib{ L"7z.dll" };
    BitCompressor compressor{ lib, BitFormat::Zip };

    std::vector< std::wstring > files = { L"path/to/file1.jpg", L"path/to/file2.pdf" };
    //creates a simple zip archive of two files
    compressor.compress( files, L"output_archive.zip" );

    std::map< std::wstring, std::wstring > files_map = { { L"path/to/file1.jpg",L"alias/path/file1.jpg" },
    { L"path/to/file2.pdf", L"alias/path/file2.pdf" } };
    //creates a zip archive with a custom directory structure
    compressor.compress( files_map, L"output_archive2.zip" );

    //compresses a directory
    compressor.compressDirectory( L"dir/path/", L"dir_archive.zip" );

    //creates an encrypted zip archive of two files
    compressor.setPassword( L"password" );
    compressor.compressFiles( files, L"protected_archive.zip" );

    //compresses a single file into a buffer
    std::vector< byte_t > buffer;
    BitCompressor compressor2{ lib, BitFormat::BZip2 };
    compressor2.compressFile( files[0], buffer );
    return 0;
}
```

### Reading archive metadata

```cpp
#include "bitarchiveinfo.hpp"

int main(){
    Bit7zLibrary lib{ L"7za.dll" };
    BitArchiveInfo arc{ lib, L"archive.7z", BitFormat::SevenZip };

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
    return 0;
}
```

A complete _**API reference**_ is available in the [wiki](https://github.com/rikyoz/bit7z/wiki/) section.

## Download
<div align="center">
<a href="https://github.com/rikyoz/bit7z/releases/latest">
<img alt="Github All Releases" src="https://img.shields.io/github/downloads/rikyoz/bit7z/total.svg?style=popout-square&label=downloads&logo=data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAAEAAAABACAYAAACqaXHeAAABk0lEQVR42uzQNwGAAAwEwPjDML0zsVLkgIkwcV8MXGRn3ea9bsvniy/btEVy/gEAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAANd9FPPylmcOiREFYRj8jpNj5iqx7WZs7GLbwj7zx2YLNRaqntRbZ+oiFLuxFUAodmnyv5I7ClUZfXmOl4HBPgZC9YqN3OQLzit4AUXYzUh+Ax9AEQ4ykN/FZ3DZUyQ1P0lY/gAPfBfhPDV5GrDkBuJHERS/SkWeFp/khN9Q+GBxKgHwH/KVfw5QEru8iQhlEQYoAWEwQkVE8mWwAEWojEC+AhahRWRVfPLmI9SEJk8DBg6hCLUBydfCB1yyBv/yrAE+EYq1e1zFbUEICMW7PMi3IiSk5tyhfAcChCJo+/K8BwFDEfotbtNLRABFGLYg34+YUFqMm5JXWowiQmgn67SB/XgziBiKMPsP+VkgfijC0h/k52EU/xHWfiG/hAShCNs/kF9DykjN939y1Cb1CCcfLOcPkBMvjz7RfeTIwFD/Ml3gkVsGJl3QH5M1zwAAAABJRU5ErkJggg=="/>
</a>
</div>

Each released package contains a _precompiled version_ of the library (both in _debug_ and _release_ mode) and the _public API headers_ that are needed to use it in your program; packages are available for both _x86_ and _x64_ architectures.

Obviously, you can also clone/download this repository and build the library by yourself (please, see the [wiki](https://github.com/rikyoz/bit7z/wiki/Building-the-library)).

## Requirements

+ **Target OS:** Windows (both x86 and x64).
+ **Compiler:** MSVC 2012 or greater (MSVC 2010 supported until v2.x).
+ **DLLs:** 7-zip DLLs (v19.00 for the GitHub release packages).

The 7-zip dlls are not shipped with bit7z but they are available at [7-zip.org](http://www.7-zip.org/).

**Note**: in order to use this library you should link your program not only with **bit7z** but also with *oleaut32* and *user32* (e.g. `-lbit7z -loleaut32 -luser32`).

**Note 2**: even if compiled with the latest version of 7-zip, **bit7z** _should_ work also with the dlls of previous versions, such as v16.04. However, it is _strongly suggested_ to use dlls with the same version.

**Note 3**: the code has been tested with MSVC 2012, 2015, 2017 and 2019.

## Building bit7z

A guide on how to build this library is available [here](https://github.com/rikyoz/bit7z/wiki/Building-the-library).

## Donations

If you have found this project useful, please consider supporting it with a small donation or buying me a coffee/beer, so that I can keep improving it!
Thank you! :)

<div align="center">

<a href='https://ko-fi.com/G2G1LS1P' target='_blank'><img height='36' style='border:0px;height:36px;' src='https://az743702.vo.msecnd.net/cdn/kofi2.png?v=0' border='0' alt='Buy Me a Coffee at ko-fi.com' /></a> [![Beerpay](https://beerpay.io/rikyoz/bit7z/badge.svg)](https://beerpay.io/rikyoz/bit7z) [![Beerpay Wish](https://beerpay.io/rikyoz/bit7z/make-wish.svg)](https://beerpay.io/rikyoz/bit7z)

[![PayPal](https://img.shields.io/badge/-Donate%20on%20PayPal-yellow.svg?logo=paypal&style=popout-square)](https://www.paypal.com/cgi-bin/webscr?cmd=_s-xclick&hosted_button_id=NTZF5G7LRXDRC)

</div>

## License (GPL v2)

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.

<br/>
<div align="center">

Copyright &copy; 2014 - 2019 Riccardo Ostani (<a href="https://github.com/rikyoz">@rikyoz</a>)

</div>
