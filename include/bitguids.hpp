#ifndef BITGUIDS_HPP
#define BITGUIDS_HPP

#include <guiddef.h>

namespace bit7z {
    extern "C" const GUID IID_IInArchive;
    extern "C" const GUID IID_IOutArchive;
    extern "C" const GUID IID_IInStream;
    extern "C" const GUID IID_IOutStream;
    extern "C" const GUID IID_IStreamGetSize;
    extern "C" const GUID IID_IStreamGetProps;
    extern "C" const GUID IID_IStreamGetProps2;
    extern "C" const GUID IID_ISequentialInStream;
    extern "C" const GUID IID_IArchiveUpdateCallback2;
    extern "C" const GUID IID_IArchiveOpenVolumeCallback;
    extern "C" const GUID IID_IArchiveOpenSetSubArchiveName;
    extern "C" const GUID IID_ICompressProgressInfo;
    extern "C" const GUID IID_ICryptoGetTextPassword;
    extern "C" const GUID IID_ICryptoGetTextPassword2;
    extern "C" const GUID IID_ISetProperties;
}
#endif // BITGUIDS_HPP
