#include <iostream>

#include <string>

#include "../include/bitextractor.hpp"
#include "../include/bitcompressor.hpp"
#include "../include/bitexception.hpp"
#include "../include/fsindexer.hpp"

using namespace std;
using namespace Bit7z;
//using namespace Bit7z::FileSystem;

static const wstring format_name[] = { L"ZIP", L"BZIP2", L"7Z", L"RAR", L"CAB", L"LZMA", L"LZMA86", L"ISO", L"TAR", L"GZIP", L"XZ"};
static const wstring extensions[] = { L".zip", L".bz2", L".7z", L".rar", L".cab", L".lzma", L".lzma86", L".iso", L".tar", L".gz", L".xz"};

static const wstring base_directory  =
    L"C:\\Users\\Oz\\Documents\\Programmazione\\Qt Projects\\Bit7z\\test\\files\\";
static const wstring in_directory     = base_directory + L"input\\";
static const wstring out_directory    = base_directory + L"output\\";
static const wstring gen_directory    = base_directory + L"generated\\";
static const wstring uncompressed_in1 = in_directory + L"italy.svg";
static const wstring uncompressed_in2 = in_directory + L"temp.xml";
static const wstring uncompressed_in3 = in_directory + L"subdir\\8.pdf";
static const wstring compressed_out   = gen_directory + L"test";
static const wstring compressed_out2  = gen_directory + L"test2";
static const wstring compressed_out3  = gen_directory + L"test3";
static const wstring compressed_out4  = gen_directory + L"test4";


void cleanup() {
    cout << "Cleaning old test files... ";
    for ( unsigned int i = BitFormat::Zip; i <= BitFormat::Xz; ++i ) {
        DeleteFile( ( compressed_out + extensions[i] ).c_str()  );
        DeleteFile( ( compressed_out2 + extensions[i] ).c_str() );
        DeleteFile( ( compressed_out3 + extensions[i] ).c_str() );
        DeleteFile( ( compressed_out4 + extensions[i] ).c_str() );
    }
    cout << "done" << endl << endl;
}

void extraction_test1( Bit7zLibrary const& lib, BitFormat format ) {
    cout << "Generic test...\t\t";
    try {
        BitExtractor extractor( lib, format );
        extractor.extract( compressed_out + extensions[format], out_directory );
        cout << "[ OK ]" << endl;
    } catch ( const BitException& e ) {
        cout << e.what() << endl;
    }
}

void compression_test1( Bit7zLibrary const& lib, BitFormat format ) {
    cout << "Generic test...\t\t";
    try {
        BitCompressor compressor( lib, format );
        compressor.compress( {in_directory + L"subdir\\", uncompressed_in1, in_directory + L"p", uncompressed_in3},
                             compressed_out + extensions[format] );
        cout << "[ OK ]" << endl;
    } catch ( const BitException& e ) {
        cout << e.what() << endl;
    }
}

void compression_test2( Bit7zLibrary const& lib, BitFormat format ) {
    cout << "Single-File test...\t";
    try {
        BitCompressor compressor( lib, format );
        compressor.compressFile( uncompressed_in3, compressed_out2 + extensions[format] );
        cout << "[ OK ]" << endl;
    } catch ( const BitException& e ) {
        cout << e.what() << endl;
    }
}

void compression_test3( Bit7zLibrary const& lib, BitFormat format ) {
    cout << "Multiple-Files test...\t";
    try {
        BitCompressor compressor( lib, format );
        compressor.compressFiles( {uncompressed_in1, uncompressed_in2, uncompressed_in3}, compressed_out3 + extensions[format] );
        cout << "[ OK ]" << endl;
    } catch ( const BitException& e ) {
        cout << e.what() << endl;
    }
}

void compression_test4( Bit7zLibrary const& lib, BitFormat format ) {
    cout << "Directory test...\t";
    try {
        BitCompressor compressor( lib, format );
        compressor.compressDirectory( in_directory + L"p\\", compressed_out4 + extensions[format] );
        cout << "[ OK ]" << endl;
    } catch ( const BitException& e ) {
        cout << e.what() << endl;
    }
}

int main() {
    try {
        cleanup();
        Bit7zLibrary lib( L"7z.dll" );
        cout << "## EXTRACTION TESTS ##" << endl;
        /*BitExtractor extractor( lib, BitFormat::SevenZip );
        extractor.extract( compressed, in_directory );
        extractor.extract( compressed_p, in_directory + L"p\\", L"qwertyuiop" );*/
        cout << "## COMPRESSION TESTS ##" << endl;
        BitFormat frmt;
        for ( unsigned int i = BitFormat::Zip; i <= BitFormat::Xz; ++i ) {
            frmt = static_cast<BitFormat>( i );
            //testing compress capabilities only for supported formats!
            if ( frmt == BitFormat::Rar || frmt == BitFormat::Cab ||
                    frmt == BitFormat::Iso || frmt == BitFormat::Lzma ||
                    frmt == BitFormat::Lzma86 ) continue;
            wcout << L"[Testing Format " << format_name[i] << L"]" << endl;
            if ( frmt != BitFormat::BZip2 && frmt != BitFormat::GZip && frmt != BitFormat::Xz )
                compression_test1( lib, frmt );
            compression_test2( lib, frmt );
            if ( frmt != BitFormat::BZip2 && frmt != BitFormat::GZip && frmt != BitFormat::Xz ) {
                compression_test3( lib, frmt );
                compression_test4( lib, frmt );
            }
            wcout << endl;
        }
    } catch ( const BitException& e ) {
        cerr << "[Errore] " << e.what() << endl;
    } catch ( ... ) {
        cerr << "[Errore] Sconosciuto..." << endl;
    }

    return 0;
}

