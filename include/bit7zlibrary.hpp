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

    enum BitFormat { Zip, BZip2, SevenZip, Rar, Cab, Lzma, Lzma86, Iso, Tar, GZip };

    class Bit7zLibrary {
            friend class BitExtractor;
            friend class BitCompressor;

        public:
            Bit7zLibrary();
            Bit7zLibrary( const std::wstring& dllPath );

        private:
            CMyComPtr<IInArchive>   inputArchiveObject( BitFormat ) const;
            CMyComPtr<IOutArchive> outputArchiveObject( BitFormat ) const;
            void createArchiveObject( BitFormat format,
                                      const GUID* interfaceID,
                                      void** outObject ) const;

            NWindows::NDLL::CLibrary _lib;
            CreateObjectFunc _createObjectFunc;
    };

}

#endif // BIT7ZLIBRARY_HPP
