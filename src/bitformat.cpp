#include "../include/bitformat.hpp"

using namespace bit7z;

BitOutFormat::BitOutFormat( unsigned int value ) : mValue( value ) {}

bool BitOutFormat::operator==( const Format& other ) const {
    return mValue == other;
}

bool BitOutFormat::operator!=( const Format& other ) const {
    return mValue != other;
}

BitOutFormat::operator unsigned int() const {
    return mValue;
}

unsigned int BitOutFormat::value() const {
    return mValue;
}

const GUID* BitOutFormat::guid() const {
    switch ( mValue ) {
        case BitOutFormat::Zip:      return &CLSID_CFormatZip;
        case BitOutFormat::BZip2:    return &CLSID_CFormatBZip2;
        case BitOutFormat::SevenZip: return &CLSID_CFormat7z;
        case BitOutFormat::Xz:       return &CLSID_CFormatXz;
        case BitOutFormat::Wim:      return &CLSID_CFormatWim;
        case BitOutFormat::Tar:      return &CLSID_CFormatTar;
        case BitOutFormat::GZip:     return &CLSID_CFormatGZip;
        default:                     return &CLSID_CFormat7z;
    }
}


BitInFormat::BitInFormat( unsigned int value ) : BitOutFormat( value ) {}

const GUID* BitInFormat::guid() const {
    switch ( mValue ) {
        case BitInFormat::Rar:      return &CLSID_CFormatRar;
        case BitInFormat::Arj:      return &CLSID_CFormatArj;
        case BitInFormat::Z:        return &CLSID_CFormatZ;
        case BitInFormat::Lzh:      return &CLSID_CFormatLzh;
        case BitInFormat::Cab:      return &CLSID_CFormatCab;
        case BitInFormat::Nsis:     return &CLSID_CFormatNsis;
        case BitInFormat::Lzma:     return &CLSID_CFormatLzma;
        case BitInFormat::Lzma86:   return &CLSID_CFormatLzma86;
        case BitInFormat::Ppmd:     return &CLSID_CFormatPpmd;
        case BitInFormat::TE:       return &CLSID_CFormatTE;
        case BitInFormat::UEFIc:    return &CLSID_CFormatUEFIc;
        case BitInFormat::UEFIs:    return &CLSID_CFormatUEFIs;
        case BitInFormat::SquashFS: return &CLSID_CFormatSquashFS;
        case BitInFormat::CramFS:   return &CLSID_CFormatCramFS;
        case BitInFormat::APM:      return &CLSID_CFormatAPM;
        case BitInFormat::Mslz:     return &CLSID_CFormatMslz;
        case BitInFormat::Flv:      return &CLSID_CFormatFlv;
        case BitInFormat::Swf:      return &CLSID_CFormatSwf;
        case BitInFormat::Swfc:     return &CLSID_CFormatSwfc;
        case BitInFormat::Ntfs:     return &CLSID_CFormatNtfs;
        case BitInFormat::Fat:      return &CLSID_CFormatFat;
        case BitInFormat::Mbr:      return &CLSID_CFormatMbr;
        case BitInFormat::Vhd:      return &CLSID_CFormatVhd;
        case BitInFormat::Pe:       return &CLSID_CFormatPe;
        case BitInFormat::Elf:      return &CLSID_CFormatElf;
        case BitInFormat::Macho:    return &CLSID_CFormatMacho;
        case BitInFormat::Udf:      return &CLSID_CFormatUdf;
        case BitInFormat::Xar:      return &CLSID_CFormatXar;
        case BitInFormat::Mub:      return &CLSID_CFormatMub;
        case BitInFormat::Hfs:      return &CLSID_CFormatHfs;
        case BitInFormat::Dmg:      return &CLSID_CFormatDmg;
        case BitInFormat::Compound: return &CLSID_CFormatCompound;
        case BitInFormat::Iso:      return &CLSID_CFormatIso;
        case BitInFormat::Chm:      return &CLSID_CFormatChm;
        case BitInFormat::Split:    return &CLSID_CFormatSplit;
        case BitInFormat::Rpm:      return &CLSID_CFormatRpm;
        case BitInFormat::Deb:      return &CLSID_CFormatDeb;
        case BitInFormat::Cpio:     return &CLSID_CFormatCpio;
        default:                    return BitOutFormat::guid();
    }
}
