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
             * @param dll_path  the path to the dll wanted
             */
            explicit Bit7zLibrary( const std::wstring& dll_path = DEFAULT_DLL );

            /**
             * @brief Destructs the Bit7zLibrary object, freeing the loaded dynamic-link library (DLL) module.
             */
            virtual ~Bit7zLibrary();

            /**
             * @brief Initiates the object needed to create a new archive or use an old one
             *
             * @note Usually this method should not be called directly by users of the bit7z library.
             *
             * @param format_ID     GUID of the archive format (see BitInFormat's guid() method)
             * @param interface_ID  ID of the archive interface to be requested (IID_IInArchive or IID_IOutArchive)
             * @param out_object    Pointer to a CMyComPtr of an object wich implements the interface requested
             */
            void createArchiveObject( const GUID* format_ID, const GUID* interface_ID, void** out_object ) const;

        private:
            typedef UINT32 ( WINAPI * CreateObjectFunc )( const GUID* clsID, const GUID* interfaceID, void** out );

            HMODULE mLibrary;
            CreateObjectFunc mCreateObjectFunc;
    };
}

#endif // BIT7ZLIBRARY_HPP
