#ifndef BIT7ZLIBRARY_HPP
#define BIT7ZLIBRARY_HPP

#include <iostream>

#include "7zip/Archive/IArchive.h"
#include "Common/MyCom.h"
#include "Windows/DLL.h"

typedef UINT32 ( WINAPI* CreateObjectFunc )( const GUID* clsID,
                                             const GUID* interfaceID,
                                             void** outObject );

namespace Bit7z {

    enum BitFormat { Zip, BZip2, SevenZip, Rar, Cab, Lzma, Lzma86, Iso, Tar, GZip, Xz };

    class Bit7zLibrary {
            friend class BitExtractor;
            friend class BitCompressor;

        public:
            Bit7zLibrary();
            Bit7zLibrary( const std::wstring& dll_path );

        private:
            CMyComPtr<IInArchive>   inputArchiveObject( BitFormat format ) const;
            CMyComPtr<IOutArchive> outputArchiveObject( BitFormat format ) const;
            void createArchiveObject( BitFormat format,
                                      const GUID* interface_ID,
                                      void** out_object ) const;

            NWindows::NDLL::CLibrary mLibrary;
            CreateObjectFunc mCreateObjectFunc;
    };

}

#endif // BIT7ZLIBRARY_HPP
