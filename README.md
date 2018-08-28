<h1 align="center">
bit7z
</h1>

<h3 align="center">A C++ static library offering a clean and simple interface to the 7-zip DLLs</h3>

<p align="center">
  <a href="https://ci.appveyor.com/project/rikyoz/bit7z">
    <img src="https://img.shields.io/appveyor/ci/rikyoz/bit7z.svg?style=flat-square&logo=appveyor" alt="Build status">
  </a>
  <a href="https://github.com/rikyoz/bit7z/releases/latest">
    <img src="https://img.shields.io/github/release/rikyoz/bit7z/all.svg?style=flat-square&logo=github&logoColor=white&colorB=blue" alt="GitHub release">
  </a>
  <img src="https://img.shields.io/badge/compiler-MSVC%202012%20--%202017-red.svg?style=flat-square&logo=visual-studio-code&logoColor=white" alt="MSVC version">
  <img src="https://img.shields.io/badge/arch-x86,%20x86__64-orange.svg?style=flat-square&logo=data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAAEAAAABACAQAAAAAYLlVAAAC1UlEQVR42u3WA9AjWRSA0bu2bdu2bdssrm3btm3btm3bmX+Ms7rLTiW9NUmvcsL7XuELu6Ojoz5DWcc5nvKp2kBdPvesi21m1Pgr7OxjrfWtgw0VZZjKM9rjXfNHM+bWWzutGo2YSH/tNm+jgNe1XzdDR322V41Tox5D6K4qY0WRtVRnjyhysercH0VeVJ13o8hXqvNNFOlSna4oUlOd2r8moBPwoQfd6THfoLweauqp6aJ8wInmMmjujWAFtwMeNJup5cXsVnWYDyDtajQjmMp7QOoypxGMbMtyAe+Ztf5/JTaJAkM6mjRXrj0KpE9zdZIyAV8bLX5lBIPlszXAVlGXMwAr5fwskL4wdPzAfGUC5o9kJy+o+dCVloiwJNg2907wimddZrqcB9GtNQF3RXI+kI5yCcgADwF6yvfLNa0JWD7n5dWXAa4lbZwrR7UioKdhc76vdEB+KxzbioAncxpGr9IBM+XKDa0IuCanaWkS8BzguEhqrQg4P6e5mgasbV+7WCySvWlFwIU5zdYooMhytCbghpzGLh9gAodCWjFXXwDSV4aJH5inWcBLkbzTOMBa9rWvk92jH5BWqBvwjSHKBfQ3as4HlvoSFq2b+zcB6bXIj6pZABvnPKzPgPSJlxV/hkUH5v7SUPiv2LN5wKuRjO82wDdON6xFSwW8XvhdcGYkrzUPYJf4lcktZh4jxg8sViqA9SKZxDo2NH0km1ImgE2jDjuBLXK6FPX1N1fUYQnKBnCeGeN3jGdPfUC+P27TyO7GjN8xoUMpHZCecKZ97etE9+hD6vKQOz1jgMa6u90J+VO9V//OaXnzgE5Al+p0iyLfqM63UeRV1Xk/ilylOo9Gkc1U55AoMrz+qjJJ1OMQ1bgq6jOYr1Rh9EgFZtd+q0QjVtFeW0UzFvGJ9uhhrSjDSE7UX6tdaMIoz0R2cbvXfKE2UJevvOEe+5kuOjr+qb4H0/HV/SQ0YjEAAAAASUVORK5CYII=" alt="Architectures">
  <a href="https://github.com/rikyoz/bit7z/blob/master/LICENSE">
    <img src="https://img.shields.io/badge/license-GNU%20GPL%20v2-lightgrey.svg?style=flat-square&logo=data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAAEAAAABACAQAAAAAYLlVAAAAvUlEQVR42u3Zt1EEURRE0Ye0gFwI4BWJoC0KGxftExk+RQATAN+nLpo1R0+v6NsJHPOLcO4vKkrPVexEcMFZN8AQ7UXwCBx0ART6VtiN4BCA1AHO+SnVAEg1AFINgFQDINUASDUAUg2AVAMg1QBINQBSDYBUAyDVAMhxAVfUdzkmYJ+7mj1xPQ7gjbWYJTmQbGsB77zy0mjPPQH9UwOKAQYYYIABBhhggAFzCTDAAAMMMGDS+v2a9V8Vzs1PH+dRolvEzoAoAAAAAElFTkSuQmCC" alt="License">
  </a>
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

**bit7z** is a C++ static library which allows to compress and extract many file archive formats,  all through a clean, simple and entirely object-oriented interface to the dynamic libraries from the 7-zip project (https://www.7-zip.org/).<br/>It supports compression and extraction to and from the filesystem or the memory, reading of archives metadata, multi-volume archives, operation progress callbacks and many other functionalities.

## Supported Features

+ **Compression** of files and directories in the following archive formats: 7z, XZ, BZIP2, GZIP, TAR, ZIP and WIM.
+ **Extraction** of the following archive formats: 7z, AR, ARJ, BZIP2, CAB, CHM, CPIO, CramFS, DEB, DMG, EXT, FAT, GPT, GZIP, HFS, HXS, IHEX, ISO, LZH, LZMA, MBR, MSI, NSIS, NTFS, QCOW2, RAR, RAR5, RPM, SquashFS, TAR, UDF, UEFI, VDI, VHD, VMDK, WIM, XAR, XZ, Z and ZIP.
+ **Reading metadata** of archives and of their content (from v3.x)
+ **Testing** archives for errors (from v3.x)
+ **Compression and extraction to and from memory** (from v2.x &mdash; compression to memory is supported only for BZIP2, GZIP, XZ and TAR formats).
+ Compression using a **custom directory system** in the output archives (from v3.x)
+ **Selective extraction** of only specified files/folders contained in archives (from v3.x)
+ Creation of **encrypted archives** (strong AES-256 encryption &mdash; only for 7z and ZIP formats).
+ **Archive header encryption** (only for 7z format).
+ Choice of the **compression level** (from none to ultra, not all supported by every output archive format).
+ **Solid archives** (only for 7z).
+ **Multi-volume archives** (from v2.1.x).
+ **Operation callbacks**, through which it is possible to obtain real time information about the ongoing extraction or compression operation (from v2.1.x).

Please note that the presence or not of some of the above features depends on the particular .dll used along with bit7z. For example, the 7z.dll should support all these features, while 7za.dll should support only the 7z file format and the 7zxa.dll can only extract 7z files. For more information about the 7z SDK DLLs, please see this [wiki page](https://github.com/rikyoz/bit7z/wiki/7z-DLLs).

## Getting Started (Library Usage)

Below are a few examples that show how to use some of the main features of bit7z:

### Extracting files from an archive

```cpp
Bit7zLibrary lib(L"7za.dll");
BitExtractor extractor(lib, BitFormat::SevenZip);

//extracts a simple archive
extractor.extract(L"path/to/archive.7z", L"output/dir/");

//extracts a specific file from the archive
extractor.extractMatching(L"path/to/archive.7z", L"file.pdf", L"output/dir/");

//extracts the first file of an archive to a buffer
vector<byte_t> buffer;
extractor.extract(L"path/to/archive.7z", buffer);

//extracts an encrypted archive
extractor.setPassword(L"password");
extractor.extract(L"path/to/another/archive.7z", L"output/dir/");
```

### Compressing files into an archive

```cpp
Bit7zLibrary lib(L"7z.dll");
BitCompressor compressor(lib, BitFormat::Zip);

std::vector<std::wstring> files = {L"path/to/file1.jpg", L"path/to/file2.pdf"};
//creates a simple zip archive of two files
compressor.compress(files, L"output_archive.zip");

std::map<std::wstring, std::wstring> files_map = {{L"path/to/file1.jpg",L"alias/path/file1.jpg"},
 {L"path/to/file2.pdf", L"alias/path/file2.pdf"}};
//creates a zip archive with a custom directory structure
compressor.compress( files_map, L"output_archive2.zip");

//compresses a directory
compressor.compressDirectory(L"dir/path/", L"dir_archive.zip");

//creates an encrypted zip archive of two files
compressor.setPassword(L"password");
compressor.compressFiles(files, L"protected_archive.zip");

//compresses a single file into a buffer
vector<byte_t> buffer;
BitCompressor compressor2(lib, BitFormat::BZip2);
compressor2.compressFile(files[0], buffer);
```

### Reading archive metadata

```cpp
Bit7zLibrary lib(L"7za.dll");
BitArchiveInfo info( lib, L"archive.7z", BitFormat::SevenZip);

//printing archive metadata
wcout << L"Archive properties" << endl;
wcout << L" Items count: "   << info.itemsCount() << endl;
wcout << L" Folders count: " << info.foldersCount() << endl;
wcout << L" Files count: "   << info.filesCount() << endl;
wcout << L" Size: "          << info.size() << endl;
wcout << L" Packed size: "   << info.packSize() << endl;
wcout << endl;

//printing archive items metadata
wcout << L"Archive items";
auto arc_items = info.items();
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
```

A complete _**API reference**_ is available in the [wiki](https://github.com/rikyoz/bit7z/wiki/) section.

## Download
<div align="center">
<a href="https://github.com/rikyoz/bit7z/releases/latest">
<img src="https://user-images.githubusercontent.com/1334291/44652398-15ec8c00-a9ec-11e8-9208-36bfebf2d2cd.png" width="64"/><br/><img alt="Github All Releases" src="https://img.shields.io/github/downloads/rikyoz/bit7z/total.svg?style=flat-square&label=downloads"/>
</a>
</div>

The released packages contain a _precompiled version_ of the library (both in _debug_ and _release_ mode) and the _public API headers_ that are needed to use it in your program; they are available for both _x86_ and _x64_ architectures.

Obviously, you can also clone/download this repository and build the library by yourself (please, see the [wiki](https://github.com/rikyoz/bit7z/wiki/Building-the-library)).

## Requirements

+ **Target OS:** Windows (both x86 and x64)
+ **Compiler:** MSVC 2012 or greater (tested with versions 2012, 2015 and 2017)
+ **DLLs:** 7-zip DLLs (v18.05)

The 7-zip dlls are not shipped with bit7z but they are available at [7-zip.org](http://www.7-zip.org/)

**Note**: in order to use this library you should link your program not only with **bit7z** but also with *oleaut32* and *user32* (e.g. `-lbit7z -loleaut32 -luser32`)

**Note 2**: even if compiled with the latest version of 7-zip, **bit7z** _should_ work also with the dlls of previous versions, such as v16.04. However, it is _strongly suggested_ to use dlls with the same version.

## Building bit7z

A guide on how to build this library is available [here](https://github.com/rikyoz/bit7z/wiki/Building-the-library).

## Donations

If you have found this project useful, please consider a small donation! Thank you! :)

<div align="center">

[![Donate](https://www.paypalobjects.com/en_US/i/btn/btn_donateCC_LG.gif)](https://www.paypal.com/cgi-bin/webscr?cmd=_s-xclick&hosted_button_id=NTZF5G7LRXDRC)

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

Copyright &copy; 2014 - 2018 Riccardo Ostani ([@rikyoz](https://github.com/rikyoz))

</div>
