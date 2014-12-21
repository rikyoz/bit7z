#ifndef BIT7ZLIBRARY_HPP
#define BIT7ZLIBRARY_HPP

#include <iostream>

#include <Windows.h>

#include "../include/bitformat.hpp"

#define DEFAULT_DLL L"7z.dll"

namespace bit7z {

    class Bit7zLibrary {
            friend class BitExtractor;
            friend class BitCompressor;

        public:
            Bit7zLibrary( const std::wstring& dll_path = DEFAULT_DLL );
            virtual ~Bit7zLibrary();

        private:
            typedef UINT32 ( WINAPI* CreateObjectFunc )( const GUID* clsID,
                                                         const GUID* interfaceID,
                                                         void** outObject );

            void createArchiveObject( const GUID* format_ID,
                                      const GUID* interface_ID,
                                      void** out_object ) const;

            HMODULE mLibrary;
            CreateObjectFunc mCreateObjectFunc;


    };

}

#endif // BIT7ZLIBRARY_HPP
