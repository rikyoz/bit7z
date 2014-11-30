#ifndef BITGUIDS_HPP
#define BITGUIDS_HPP

#include <guiddef.h>

namespace Bit7z {
    extern const GUID CLSID_CFormatZip;
    extern const GUID CLSID_CFormatBZip2;
    extern const GUID CLSID_CFormat7z;
    extern const GUID CLSID_CFormatRar;
    extern const GUID CLSID_CFormatCab;
    extern const GUID CLSID_CFormatLzma;
    extern const GUID CLSID_CFormatLzma86;
    extern const GUID CLSID_CFormatIso;
    extern const GUID CLSID_CFormatTar;
    extern const GUID CLSID_CFormatGZip;

    extern "C" const GUID IID_IInArchive;
    extern "C" const GUID IID_IOutArchive;
    extern "C" const GUID IID_IInStream;
    extern "C" const GUID IID_IOutStream;
    extern "C" const GUID IID_IStreamGetSize;
    extern "C" const GUID IID_IArchiveUpdateCallback2;
    extern "C" const GUID IID_ICryptoGetTextPassword;
    extern "C" const GUID IID_ICryptoGetTextPassword2;
}

#endif // BITGUIDS_HPP
