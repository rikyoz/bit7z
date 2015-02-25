#ifndef BIT7ZLIBRARY_HPP
#define BIT7ZLIBRARY_HPP

#include <iostream>

#include <Windows.h>

#include "../include/bitformat.hpp"

#define DEFAULT_DLL L"7z.dll"

namespace bit7z {

    /**
     * @brief The Bit7zLibrary class allows the access to the basic functionalities provided by the 7z DLLs.
     */
    class Bit7zLibrary {
            friend class BitExtractor;
            friend class BitCompressor;
            friend class BitMemExtractor;
            friend class BitMemCompressor;

        public:
            /**
             * @brief Constructs a Bit7zLibrary object using the path of the wanted 7zip DLL.
             *
             * By default, it searches a 7z.dll in the same path of the application.
             *
             * @param dll_path the path to the dll wanted
             */
            Bit7zLibrary( const std::wstring& dll_path = DEFAULT_DLL );
            virtual ~Bit7zLibrary();

        private:
            typedef UINT32 ( WINAPI* CreateObjectFunc )( const GUID* clsID, const GUID* interfaceID, void** outObject );

            void createArchiveObject( const GUID* format_ID, const GUID* interface_ID, void** out_object ) const;

            HMODULE mLibrary;
            CreateObjectFunc mCreateObjectFunc;


    };

}

#endif // BIT7ZLIBRARY_HPP
