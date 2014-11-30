#include <iostream>

#include "../include/bitextractor.hpp"
#include "../include/bitcompressor.hpp"
#include "../include/bitexception.hpp"

using namespace std;

static const wstring directory       = L"C:\\Users\\Oz\\Documents\\Programmazione\\Qt Projects\\Bit7z\\test\\";
static const wstring compressed      = directory + L"NuovoREGX.regx";
static const wstring compressed_p    = directory + L"NuovoREGXP.regx";
static const wstring compressed_out  = directory + L"test.7z";
static const wstring compressed_out2 = directory + L"test2.7z";
static const wstring uncompressed1   = directory + L"italy.svg";
static const wstring uncompressed2   = directory + L"8.pdf";

int main() {
    using namespace Bit7z;

    try {
        Bit7zLibrary lib(L"7z.dll");
        BitExtractor extractor(lib, BitFormat::SevenZip);
        extractor.extract(compressed, directory);
        extractor.extract(compressed_p, directory + L"/p/", L"qwertyuiop");
        BitCompressor compressor(lib, BitFormat::SevenZip);
        compressor.setPassword(L"ciao", true);
        compressor.compress({uncompressed1, uncompressed2}, compressed_out2);
    } catch ( const BitException &e ) {
        cerr << "[Errore] " << e.what() << endl;
    } catch ( ... ) {
        cerr << "[Errore] Sconosciuto..." << endl;
    }

    return 0;
}

