#include <iostream>

#include <string>

#include "../include/bitextractor.hpp"
#include "../include/bitcompressor.hpp"
#include "../include/bitexception.hpp"
#include "../include/fsindexer.hpp"

using namespace std;

static const wstring directory       =
    L"C:\\Users\\Oz\\Documents\\Programmazione\\Qt Projects\\Bit7z\\test\\";
static const wstring compressed      = directory + L"NuovoREGX.regx";
static const wstring compressed_p    = directory + L"NuovoREGXP.regx";
static const wstring compressed_out  = directory + L"test.7z";
static const wstring compressed_out2 = directory + L"test2.7z";
static const wstring compressed_out3 = directory + L"test3.7z";
static const wstring compressed_out4 = directory + L"test4.7z";
static const wstring uncompressed1   = directory + L"italy.svg";
static const wstring uncompressed2   = directory + L"subdir\\8.pdf";

int main() {
    using namespace Bit7z;
    using namespace Bit7z::FileSystem;

    try {
        DeleteFile( compressed_out.c_str()  );
        DeleteFile( compressed_out2.c_str() );
        DeleteFile( compressed_out3.c_str() );
        DeleteFile( compressed_out4.c_str() );
        Bit7zLibrary lib( L"7z.dll" );
        BitExtractor extractor( lib, BitFormat::SevenZip );
        extractor.extract( compressed, directory );
        extractor.extract( compressed_p, directory + L"p\\", L"qwertyuiop" );
        BitCompressor compressor( lib, BitFormat::SevenZip );
        //compressor.setPassword( L"ciao", true );
        compressor.compressFile( uncompressed2, compressed_out );
        compressor.compressFiles( {uncompressed1, uncompressed2}, compressed_out2 );
        compressor.compressDirectory( directory + L"p\\", compressed_out3 );
        compressor.compress( {directory + L"subdir", uncompressed1, directory + L"p", uncompressed2},
                             compressed_out4 );
        /*compressor.setPassword( L"", false );
        compressor.compressDirectory( directory, compressed_out3 );
        compressor.compress( {directory + L"subdir\\", uncompressed1, directory + L"p\\", uncompressed2},
                             compressed_out4 );*/
        /*vector<FSItem> result;
        FSIndexer indexer( directory + L"p\\" );
        indexer.listFilesInDirectory( result );
        cout << "results: " << result.size() << endl;
        for ( FSItem item : result )
            wcout << item.relativePath() << endl;*/
    } catch ( const BitException& e ) {
        cerr << "[Errore] " << e.what() << endl;
    } catch ( ... ) {
        cerr << "[Errore] Sconosciuto..." << endl;
    }

    return 0;
}

