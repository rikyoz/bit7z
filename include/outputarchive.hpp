#ifndef OUTPUTARCHIVE_HPP
#define OUTPUTARCHIVE_HPP

#include "../include/bitarchivecreator.hpp"
#include "../include/bitinputarchive.hpp"
#include "../include/bitexception.hpp"
#include "../include/compresscallback.hpp"
#include "../include/fsitem.hpp"

#include <vector>

#include "7zip/Archive/IArchive.h"
#include "Common/MyCom.h"

namespace bit7z {
    using std::unique_ptr;
    using std::vector;
    using namespace filesystem;

    class OutputArchive {
        public:
            explicit OutputArchive( const BitArchiveCreator& creator );

            void compress( const vector< FSItem >& in_items, const wstring& out_archive );

            void compress( const vector< FSItem >& in_items, vector< byte_t >& out_buffer );

            void compress( const vector< byte_t >& in_buffer, const wstring& in_buffer_name, const wstring& out_archive );

            void compress( const vector< byte_t >& in_buffer, const wstring& in_buffer_name, vector< byte_t >& out_buffer );

            virtual ~OutputArchive() {}

        private:
            CMyComPtr< IOutStream > initOutFileStream( const wstring& out_archive, unique_ptr< BitInputArchive >& old_arc );

            HRESULT compress( ISequentialOutStream* out_stream, CompressCallback* update_callback );

            void cleanupOldArc( BitInputArchive* old_arc, IOutStream* out_file_stream, const wstring& out_archive );

            const BitArchiveCreator& mCreator;
            CMyComPtr< IOutArchive > mOutArc;
    };
}

#endif // OUTPUTARCHIVE_HPP
