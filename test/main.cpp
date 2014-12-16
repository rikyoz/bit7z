#include <iostream>
#include <map>
#include <string>

#include "../include/bitextractor.hpp"
#include "../include/bitcompressor.hpp"
#include "../include/bitexception.hpp"

using namespace std;
using namespace Bit7z;

static const wstring format_name[] = { L"ZIP", L"BZIP2", L"7Z", L"XZ", L"WIM", L"TAR", L"GZIP"};
static const wstring extensions[]  = { L".zip", L".bz2", L".7z", L".xz", L".wim", L".tar", L".gz" };

static const wstring test_directory   = L"..\\..\\files\\";
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

void compression_test1( BitCompressor const& compressor, BitOutFormat format ) {
    cout << "Generic test...\t\t";
    try {
        compressor.compress( {in_directory + L"subdir\\", uncompressed_in1, in_directory + L"p", uncompressed_in3},
                             compressed_out + extensions[format.value()] );
        cout << "[ OK ]" << endl;
    } catch ( const BitException& e ) {
        cout << e.what() << endl;
    }
}

void compression_test2( BitCompressor const& compressor, BitOutFormat format ) {
    cout << "Single-File test...\t";
    try {
        compressor.compressFile( uncompressed_in3, compressed_out2 + extensions[format.value()] );
        cout << "[ OK ]" << endl;
    } catch ( const BitException& e ) {
        cout << e.what() << endl;
    }
}

void compression_test3( BitCompressor const& compressor, BitOutFormat format ) {
    cout << "Multiple-Files test...\t";
    try {
        compressor.compressFiles( {uncompressed_in1, uncompressed_in2, uncompressed_in3}, compressed_out3 +
                                  extensions[format.value()] );
        cout << "[ OK ]" << endl;
    } catch ( const BitException& e ) {
        cout << e.what() << endl;
    }
}

void compression_test4( BitCompressor const& compressor, BitOutFormat format ) {
    cout << "Directory test...\t";
    try {
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
        for ( unsigned int i = BitOutFormat::Zip; i < BitOutFormat::GZip; ++i ) {
            BitOutFormat frmt = i;
            BitCompressor compressor( lib, frmt );
            //compressor.setPassword( L"qwertyuiop", true );
            //compressor.setCompressionLevel( BitCompressionLevel::Ultra );
            //compressor.setSolidMode( true );
            //testing compress capabilities only for supported formats!
            wcout << L"[Testing Format " << format_name[i] << L"]" << endl;
            if ( frmt != BitOutFormat::BZip2 && frmt != BitOutFormat::GZip && frmt != BitOutFormat::Xz )
                compression_test1( compressor, frmt );
            compression_test2( compressor, frmt );
            if ( frmt != BitOutFormat::BZip2 && frmt != BitOutFormat::GZip && frmt != BitOutFormat::Xz ) {
                compression_test3( compressor, frmt );
                compression_test4( compressor, frmt );
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

