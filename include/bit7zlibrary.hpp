#ifndef BIT7ZLIBRARY_HPP
#define BIT7ZLIBRARY_HPP

#include <iostream>

#include <Windows.h>

#include "../include/bitformat.hpp"

typedef UINT32 ( WINAPI* CreateObjectFunc )( const GUID* clsID,
                                             const GUID* interfaceID,
                                             void** outObject );

namespace bit7z {

    class Bit7zLibrary {
            friend class BitExtractor;
            friend class BitCompressor;

        public:
            Bit7zLibrary();
            Bit7zLibrary( const std::wstring& dll_path );

        private:
            void createArchiveObject( const GUID* format_ID,
                                      const GUID* interface_ID,
                                      void** out_object ) const;

            HMODULE mLibrary;
            CreateObjectFunc mCreateObjectFunc;
    };

}

#endif // BIT7ZLIBRARY_HPP
