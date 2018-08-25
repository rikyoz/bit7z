// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#include "../include/bitpropvariant.hpp"

#include "Windows/PropVariant.h"

#include "../include/bitexception.hpp"

using namespace bit7z;

BitPropVariantType lookupType( VARTYPE type ) {
    switch ( type ) {
        case VT_EMPTY:
            return BitPropVariantType::Empty;
        case VT_BOOL:
            return BitPropVariantType::Bool;
        case VT_BSTR:
            return BitPropVariantType::String;
        case VT_UI1:
            return BitPropVariantType::Uint8;
        case VT_UI2:
            return BitPropVariantType::Uint16;
        case VT_UI4:
        case VT_UINT:
            return BitPropVariantType::Uint32;
        case VT_UI8:
            return BitPropVariantType::Uint64;
        case VT_I1:
            return BitPropVariantType::Int8;
        case VT_I2:
            return BitPropVariantType::Int16;
        case VT_I4:
        case VT_INT:
            return BitPropVariantType::Int32;
        case VT_I8:
            return BitPropVariantType::Int64;
        case VT_FILETIME:
            return BitPropVariantType::Filetime;
        default:
            // this is very unlikely to happen:
            // properties types used in archives are the ones supported by PropertyType enum class
            throw BitException( L"Property type not supported" );
    }
}

BitPropVariant::BitPropVariant() : PROPVARIANT() {
    /* As in CPropVariant default constructor (Note: it seems that the default vt value is VT_NULL)*/
    vt = VT_EMPTY;
    wReserved1 = 0;
}

BitPropVariant::BitPropVariant( const BitPropVariant& other ) : PROPVARIANT( other ) {
    if ( vt == VT_BSTR ) { //until now, I've copied only the pointer to the string, hence we need a copy!
        bstrVal = SysAllocStringByteLen( reinterpret_cast<LPCSTR>( other.bstrVal ),
                                         SysStringByteLen( other.bstrVal ) );
        if ( !bstrVal ) {
            throw BitException( L"Could not allocate memory for BitPropVariant string" );
        }
    }
}

BitPropVariant::BitPropVariant( BitPropVariant&& other ) NOEXCEPT : PROPVARIANT( other ) {
    if ( vt == VT_BSTR ) { //this and other share the pointer to the same string, but now the string belongs to this!
        other.bstrVal = nullptr;
    }
}

BitPropVariant::~BitPropVariant() {
    if ( vt == VT_BSTR && bstrVal != nullptr ) {
        ::SysFreeString( bstrVal );
    }
}

BitPropVariant& BitPropVariant::operator=( const BitPropVariant& other ) NOEXCEPT {
    BitPropVariant tmp( other ); //copy construct a tmp variable
    *this = std::move( tmp ); //move assign to this
    return *this;
}

BitPropVariant& BitPropVariant::operator=( BitPropVariant&& other ) NOEXCEPT {
    if ( this != &other ) {
        if ( vt == VT_BSTR && bstrVal != nullptr ) {
            ::SysFreeString( bstrVal ); //this was a string: since it is not needed anymore, we must free it!
        }
        vt = other.vt;
        switch ( vt ) {
            case VT_BOOL:
                boolVal = other.boolVal;
                break;
            case VT_BSTR:
                bstrVal = other.bstrVal;
                other.bstrVal = nullptr;
                break;
            case VT_UI1:
                bVal = other.bVal;
                break;
            case VT_UI2:
                uiVal = other.uiVal;
                break;
            case VT_UINT:
                uintVal = other.uintVal;
                break;
            case VT_UI4:
                ulVal = other.uiVal;
                break;
            case VT_UI8:
                uhVal = other.uhVal;
                break;
            case VT_I1:
                cVal = other.cVal;
                break;
            case VT_I2:
                iVal = other.iVal;
                break;
            case VT_INT:
                intVal = other.intVal;
                break;
            case VT_I4:
                lVal = other.lVal;
                break;
            case VT_I8:
                hVal = other.hVal;
                break;
            case VT_FILETIME:
                filetime = other.filetime;
                break;
        }
    }
    return *this;
}

bool BitPropVariant::getBool() const {
    if ( vt != VT_BOOL ) {
        throw BitException( L"BitPropVariant is not a bool" );
    }
    return boolVal != VARIANT_FALSE; //simply returning boolVal should work but this prevents some compiler warnings
}

wstring BitPropVariant::getString() const {
    if ( vt != VT_BSTR ) {
        throw BitException( L"BitPropVariant is not a string" );
    }
    return wstring( bstrVal, SysStringLen( bstrVal ) );
}

uint8_t BitPropVariant::getUInt8() const {
    switch ( vt ) {
        case VT_UI1:
            return bVal;
        default:
            throw BitException( L"BitPropVariant is not a 8-bits unsigned integer" );
    }
}

uint16_t BitPropVariant::getUInt16() const {
    switch ( vt ) {
        case VT_UI1:
            return bVal;
        case VT_UI2:
            return uiVal;
        default:
            throw BitException( L"BitPropVariant is not a 16-bits unsigned integer" );
    }
}

uint32_t BitPropVariant::getUInt32() const {
    switch ( vt ) {
        case VT_UI1:
            return bVal;
        case VT_UI2:
            return uiVal;
        case VT_UINT:
            return uintVal;
        case VT_UI4:
            return ulVal;
        default:
            throw BitException( L"BitPropVariant is not a 32-bits unsigned integer" );
    }
}

uint64_t BitPropVariant::getUInt64() const {
    switch ( vt ) {
        case VT_UI1:
            return bVal;
        case VT_UI2:
            return uiVal;
        case VT_UINT:
            return uintVal;
        case VT_UI4:
            return ulVal;
        case VT_UI8:
            return uhVal.QuadPart;
        default:
            throw BitException( L"BitPropVariant is not a 64-bits unsigned integer" );
    }
}

int8_t BitPropVariant::getInt8() const {
    switch ( vt ) {
        case VT_I1:
            return cVal;
        default:
            throw BitException( L"BitPropVariant is not a 8-bits integer" );
    }
}

int16_t BitPropVariant::getInt16() const {
    switch ( vt ) {
        case VT_I1:
            return cVal;
        case VT_I2:
            return iVal;
        default:
            throw BitException( L"BitPropVariant is not a 16-bits integer" );
    }
}

int32_t BitPropVariant::getInt32() const {
    switch ( vt ) {
        case VT_I1:
            return cVal;
        case VT_I2:
            return iVal;
        case VT_INT:
            return intVal;
        case VT_I4:
            return lVal;
        default:
            throw BitException( L"BitPropVariant is not a 32-bits integer" );
    }
}

int64_t BitPropVariant::getInt64() const {
    switch ( vt ) {
        case VT_I1:
            return cVal;
        case VT_I2:
            return iVal;
        case VT_INT:
            return intVal;
        case VT_I4:
            return lVal;
        case VT_I8:
            return hVal.QuadPart;
        default:
            throw BitException( L"BitPropVariant is not a 64-bits integer" );
    }
}

FILETIME BitPropVariant::getFiletime() const {
    if ( vt != VT_FILETIME ) {
        throw BitException( L"BitPropVariant is not a FILETIME" );
    }
    return filetime;
}

wstring BitPropVariant::toString() const {
    switch ( vt ) {
        case VT_BOOL:
            return boolVal ? L"true" : L"false";
        case VT_BSTR:
            return wstring( bstrVal, SysStringLen( bstrVal ) );
        case VT_UI1:
            return std::to_wstring( bVal );
        case VT_UI2:
            return std::to_wstring( uiVal );
        case VT_UINT:
            return std::to_wstring( uintVal );
        case VT_UI4:
            return std::to_wstring( ulVal );
        case VT_UI8:
            return std::to_wstring( uhVal.QuadPart );
        case VT_I1:
            return std::to_wstring( cVal );
        case VT_I2:
            return std::to_wstring( iVal );
        case VT_INT:
            return std::to_wstring( intVal );
        case VT_I4:
            return std::to_wstring( lVal );
        case VT_I8:
            return std::to_wstring( hVal.QuadPart );
        case VT_FILETIME:
            return std::to_wstring( filetime.dwHighDateTime ) + L", " + std::to_wstring( filetime.dwLowDateTime );
    }
    throw BitException( L"BitPropVariant type not supported (vt: " + std::to_wstring( vt ) + L")" );
}

bool BitPropVariant::isEmpty() const {
    return vt == VT_EMPTY;
}

BitPropVariantType BitPropVariant::type() const {
    return lookupType( vt );
}
