#include <iostream>
#include <map>
#include <string>

#include "../include/bitextractor.hpp"
#include "../include/bitcompressor.hpp"
#include "../include/bitexception.hpp"
#include "../include/fsindexer.hpp"

#define BASE_DIR L"C:\\Users\\Oz\\Documents\\Programmazione\\Qt Projects\\Bit7z\\"

using namespace std;
using namespace Bit7z;
//using namespace Bit7z::FileSystem;

static const wstring format_name[] = { L"ZIP", L"BZIP2", L"7Z", L"XZ", L"WIM", L"TAR", L"GZIP"};
static const wstring extensions[]  = { L".zip", L".bz2", L".7z", L".xz", L".wim", L".tar", L".gz" };

static const wstring test_directory   = BASE_DIR L"test\\files\\";
static const wstring in_directory     = test_directory + L"input\\";
static const wstring out_directory    = test_directory + L"output\\";
static const wstring gen_directory    = test_directory + L"generated\\";
static const wstring uncompressed_in1 = in_directory + L"italy.svg";
static const wstring uncompressed_in2 = in_directory + L"temp.xml";
static const wstring uncompressed_in3 = in_directory + L"subdir\\8.pdf";
static const wstring compressed_out   = gen_directory + L"test";
static const wstring compressed_out2  = gen_directory + L"test2";
static const wstring compressed_out3  = gen_directory + L"test3";
static const wstring compressed_out4  = gen_directory + L"test4";

void cleanup() {
    cout << "Cleaning old test files... ";
    for ( unsigned int i = BitOutFormat::Zip; i <= BitOutFormat::GZip; ++i ) {
        DeleteFile( ( compressed_out + extensions[i] ).c_str()  );
        DeleteFile( ( compressed_out2 + extensions[i] ).c_str() );
        DeleteFile( ( compressed_out3 + extensions[i] ).c_str() );
        DeleteFile( ( compressed_out4 + extensions[i] ).c_str() );
    }
    cout << "done" << endl << endl;
}

void extraction_test1( Bit7zLibrary const& lib, BitInFormat format ) {
    cout << "Generic test...\t\t";
    try {
        BitExtractor extractor( lib, format );
        //wcout << L"(dir: " << ( compressed_out + extensions[format] ) << L")" << endl;
        extractor.extract( compressed_out + extensions[format], out_directory );
        cout << "[ OK ]" << endl;
    } catch ( const BitException& e ) {
        cout << e.what() << endl;
    } catch ( ... ) {
        cout << "[Unknown Error]" << endl;
    }
}

void compression_test1( Bit7zLibrary const& lib, BitOutFormat format ) {
    cout << "Generic test...\t\t";
    try {
        BitCompressor compressor( lib, format );
        compressor.compress( {in_directory + L"subdir\\", uncompressed_in1, in_directory + L"p", uncompressed_in3},
                             compressed_out + extensions[format.value()] );
        cout << "[ OK ]" << endl;
    } catch ( const BitException& e ) {
        cout << e.what() << endl;
    }
}

void compression_test2( Bit7zLibrary const& lib, BitOutFormat format ) {
    cout << "Single-File test...\t";
    try {
        BitCompressor compressor( lib, format );
        compressor.compressFile( uncompressed_in3, compressed_out2 + extensions[format.value()] );
        cout << "[ OK ]" << endl;
    } catch ( const BitException& e ) {
        cout << e.what() << endl;
    }
}

void compression_test3( Bit7zLibrary const& lib, BitOutFormat format ) {
    cout << "Multiple-Files test...\t";
    try {
        BitCompressor compressor( lib, format );
        compressor.compressFiles( {uncompressed_in1, uncompressed_in2, uncompressed_in3}, compressed_out3 +
                                  extensions[format.value()] );
        cout << "[ OK ]" << endl;
    } catch ( const BitException& e ) {
        cout << e.what() << endl;
    }
}

void compression_test4( Bit7zLibrary const& lib, BitOutFormat format ) {
    cout << "Directory test...\t";
    try {
        BitCompressor compressor( lib, format );
        compressor.compressDirectory( in_directory + L"p\\", compressed_out4 + extensions[format.value()] );
        cout << "[ OK ]" << endl;
    } catch ( const BitException& e ) {
        cout << e.what() << endl;
    }
}

int main() {
    try {
        cleanup();
        Bit7zLibrary lib( L"7z.dll" );
        cout << "## COMPRESSION TESTS ##" << endl;
        for ( unsigned int i = BitOutFormat::Zip; i <= BitOutFormat::GZip; ++i ) {
            BitOutFormat frmt = i;
            //testing compress capabilities only for supported formats!
            wcout << L"[Testing Format " << format_name[i] << L"]" << endl;
            if ( frmt != BitOutFormat::BZip2 && frmt != BitOutFormat::GZip && frmt != BitOutFormat::Xz )
                compression_test1( lib, frmt );
            compression_test2( lib, frmt );
            if ( frmt != BitOutFormat::BZip2 && frmt != BitOutFormat::GZip && frmt != BitOutFormat::Xz ) {
                compression_test3( lib, frmt );
                compression_test4( lib, frmt );
            }
            wcout << endl;
        }
        cout << "## EXTRACTION TESTS ##" << endl;
        //extraction_test1( lib, BitOutFormat::Zip );
        //extraction_test1( lib, BitOutFormat::SevenZip );
        //extraction_test1( lib, BitOutFormat::Wim );
        //extraction_test1( lib, BitInFormat::Tar );
        /*BitExtractor extractor( lib, BitInFormat::SevenZip );
        extractor.extract( compressed_out, in_directory );
        extractor.extract( compressed_out2, in_directory + L"p\\", L"qwertyuiop" );*/
    } catch ( const BitException& e ) {
        cerr << "[Errore] " << e.what() << endl;
    } catch ( ... ) {
        cerr << "[Errore] Sconosciuto..." << endl;
    }

    return 0;
}

