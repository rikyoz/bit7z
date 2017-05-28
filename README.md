bit7z [![Build status](https://ci.appveyor.com/api/projects/status/5la21g1kb7ikm82n?svg=true)](https://ci.appveyor.com/project/rikyoz/bit7z) [![GitHub release](https://img.shields.io/github/release/rikyoz/bit7z.png)](https://github.com/rikyoz/bit7z/releases/latest) ![](https://img.shields.io/badge/compiler-MSVC%202010%20--%202015-red.png) ![](https://img.shields.io/badge/arch-x86,%20x86__64-orange.png?style=flat) [![](https://img.shields.io/badge/license-GNU%20GPL%20v2-lightgrey.png?style=flat)](https://github.com/rikyoz/bit7z/blob/master/LICENSE) [![donate](https://img.shields.io/donate/PayPal.png?color=yellow)](https://www.paypal.com/cgi-bin/webscr?cmd=_s-xclick&hosted_button_id=NTZF5G7LRXDRC)
=====

**bit7z** is a C++ static library which allows to compress and extract many file archive formats,  all through a clean, simple and entirely object-oriented interface to the dynamic libraries from the 7-zip project. It supports compression and extraction to and from the filesystem or the memory, multi-volume archives and operation progress callbacks.

## Features
bit7z supports the following features:

+ **Compression of files and directories** with the following archive formats: 7z, XZ, BZIP2, GZIP, TAR, ZIP and WIM.
+ **Extraction** of the following archive formats: 7z, AR, ARJ, BZIP2, CAB, CHM, CPIO, CramFS, DEB, DMG, EXT, FAT, GPT, GZIP, HFS, HXS, IHEX, ISO, LZH, LZMA, MBR, MSI, NSIS, NTFS, QCOW2, RAR, RAR5, RPM, SquashFS, TAR, UDF, UEFI, VDI, VHD, VMDK, WIM, XAR, XZ, Z and ZIP.
+ Support to **compression and extraction to and from memory** (from v2.x &mdash; compression to memory is supported only for BZIP2, GZIP, XZ and TAR formats).
+ Creation of **encrypted archives** (strong AES-256 encryption &mdash; only for 7z and ZIP formats).
+ **Archive header encryption** (only for 7z format).
+ Choice of the **compression level** (from none to ultra, not all supported by every output archive format).
+ **Solid archives** (only for 7z).
+ **Multi-volume archives** (from v2.1.x).
+ Support to **operation callbacks**, through which it is possible to obtain real time information about the ongoing extraction or compression operation (from v2.1.x).

Please note that the presence or not of some of the above features depends on the particular .dll used along with bit7z. For example, the 7z.dll should support all these features, while 7za.dll should support only the 7z file format and the 7zxa.dll can only extract 7z files. For more information about the 7z SDK DLLs, please see this [wiki page](https://github.com/rikyoz/bit7z/wiki/7z-DLLs).

## Getting Started (Library Usage)

Below are a few examples that show how to use some of the main features of bit7z:

### Extracting files from an archive
```cpp
Bit7zLibrary lib(L"7za.dll");
BitExtractor extractor(lib, BitFormat::SevenZip);

//extracts a simple archive
extractor.extract(L"path/to/archive.7z", L"output/dir/");

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
compressor.compressFiles(files, L"output_archive.zip");

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

A complete _**API reference**_ is available in the [wiki](https://github.com/rikyoz/bit7z/wiki/) section.

## Downloads

<div align="center">

### [Download Latest Version](https://github.com/rikyoz/bit7z/releases/latest)

[![Github All Releases](https://img.shields.io/github/downloads/rikyoz/bit7z/total.png)]()

</div>

The release packages contain a precompiled version of the library and they are available for both x86 and x64 architectures.

Obviously, you can also clone/download repository this repository and build the library by yourself (please, see the [wiki](https://github.com/rikyoz/bit7z/wiki/Building-the-library)).

## Usage Requirements
+ **Target OS:** Windows (both x86 and x64)
+ **Compiler:** MSVC (tested with versions 2010, 2013 and 2015)
+ **DLLs:** 7-zip DLLs (v16.04)

The 7zip dlls are not shipped with bit7z but they are available at [7-zip.org](http://www.7-zip.org/)

**Note**: in order to use this library you should link your program not only with **bit7z** but also with *oleaut32* and *user32* (e.g. `-lbit7z -loleaut32 -luser32`)

**Note 2**: even if compiled with latest version of 7-zip, **bit7z** _should_ work also with the dlls of previous versions, such as 9.20. However, it is strongly suggested to use dlls with the same version.

## Building bit7z

A complete guide on how to build this library is available [here](https://github.com/rikyoz/bit7z/wiki/Building-the-library).

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

Copyright &copy; 2014-2017 Riccardo Ostani ([@rikyoz](https://github.com/rikyoz))

</div>
