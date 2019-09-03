// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

/*
 * bit7z - A C++ static library to interface with the 7-zip DLLs.
 * Copyright (c) 2014-2019  Riccardo Ostani - All Rights Reserved.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * Bit7z is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with bit7z; if not, see https://www.gnu.org/licenses/.
 */

#include "../include/bitpropvariant.hpp"

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
            return BitPropVariantType::UInt8;
        case VT_UI2:
            return BitPropVariantType::UInt16;
        case VT_UI4:
        case VT_UINT:
            return BitPropVariantType::UInt32;
        case VT_UI8:
            return BitPropVariantType::UInt64;
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
            throw BitException( "Property type not supported", E_INVALIDARG );
    }
}

BitPropVariant::BitPropVariant() : PROPVARIANT() {
    /* As in CPropVariant default constructor (Note: it seems that the default vt value is VT_NULL)*/
    vt = VT_EMPTY;
    wReserved1 = 0;
    bstrVal = nullptr;
}

BitPropVariant::BitPropVariant( const BitPropVariant& other ) : PROPVARIANT( other ) {
    if ( vt == VT_BSTR ) { //until now, I've copied only the pointer to the string, hence we need a copy!
        bstrVal = SysAllocStringByteLen( reinterpret_cast< LPCSTR >( other.bstrVal ),
                                         SysStringByteLen( other.bstrVal ) );
        if ( !bstrVal ) {
            throw BitException( "Could not allocate memory for BitPropVariant string" );
        }
    }
}

BitPropVariant::BitPropVariant( BitPropVariant&& other ) NOEXCEPT : PROPVARIANT( other ) {
    if ( vt == VT_BSTR ) {
        /* this and other share the pointer to the same string, but now the string belongs to this!
         * Hence, if we set the other.bstrVal to nullptr, we prevent the bstrVal from being destroyed when
         * the other object is deleted! */
        other.bstrVal = nullptr;
    }
}

BitPropVariant::BitPropVariant( bool value ) : PROPVARIANT() {
    vt = VT_BOOL;
    wReserved1 = 0;
    boolVal = ( value ? VARIANT_TRUE : VARIANT_FALSE );
}

BitPropVariant::BitPropVariant( const wchar_t* value ) : PROPVARIANT() {
    vt = VT_BSTR;
    wReserved1 = 0;
    if ( value != nullptr ) {
        bstrVal = ::SysAllocString( value );
        if ( !bstrVal ) {
            throw BitException( "Could not allocate memory for BitPropVariant string" );
        }
    } else {
        bstrVal = nullptr;
    }
}

BitPropVariant::BitPropVariant( const wstring& value ) : PROPVARIANT() {
    vt = VT_BSTR;
    wReserved1 = 0;
    bstrVal = ::SysAllocStringLen( value.data(), static_cast< unsigned int >( value.size() ) );
    if ( !bstrVal ) {
        throw BitException( "Could not allocate memory for BitPropVariant string" );
    }
}

BitPropVariant::BitPropVariant( uint8_t value ) : PROPVARIANT() {
    vt = VT_UI1;
    wReserved1 = 0;
    bVal = value;
}

BitPropVariant::BitPropVariant( uint16_t value ) : PROPVARIANT() {
    vt = VT_UI2;
    wReserved1 = 0;
    uiVal = value;
}

BitPropVariant::BitPropVariant( uint32_t value ) : PROPVARIANT() {
    vt = VT_UI4;
    wReserved1 = 0;
    ulVal = value;
}

BitPropVariant::BitPropVariant( uint64_t value ) : PROPVARIANT() {
    vt = VT_UI8;
    wReserved1 = 0;
    uhVal.QuadPart = value;
}

BitPropVariant::BitPropVariant( int8_t value ) : PROPVARIANT() {
    vt = VT_I1;
    wReserved1 = 0;
    cVal = value;
}

BitPropVariant::BitPropVariant( int16_t value ) : PROPVARIANT() {
    vt = VT_I2;
    wReserved1 = 0;
    iVal = value;
}

BitPropVariant::BitPropVariant( int32_t value ) : PROPVARIANT() {
    vt = VT_I4;
    wReserved1 = 0;
    lVal = value;
}

BitPropVariant::BitPropVariant( int64_t value ) : PROPVARIANT() {
    vt = VT_I8;
    wReserved1 = 0;
    hVal.QuadPart = value;
}

BitPropVariant::BitPropVariant( const FILETIME& value ) : PROPVARIANT() {
    vt = VT_FILETIME;
    wReserved1 = 0;
    filetime = value;
}

BitPropVariant::~BitPropVariant() {
    internalClear();
}

BitPropVariant& BitPropVariant::operator=( const BitPropVariant& other ) NOEXCEPT {
    BitPropVariant tmp( other ); //copy construct a tmp variable
    *this = std::move( tmp ); //move assign to this
    return *this;
}

BitPropVariant& BitPropVariant::operator=( BitPropVariant&& other ) NOEXCEPT {
    if ( this != &other ) {
        internalClear();
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
                ulVal = other.ulVal;
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
        throw BitException( "BitPropVariant is not a bool" );
    }
    return boolVal != VARIANT_FALSE; //simply returning boolVal should work but this prevents some compiler warnings
}

wstring BitPropVariant::getString() const {
    if ( vt != VT_BSTR ) {
        throw BitException( "BitPropVariant is not a string" );
    }
    //Note: a nullptr BSTR is semantically equivalent to an empty string!
    return bstrVal == nullptr ? L"" : wstring( bstrVal, SysStringLen( bstrVal ) );
}

uint8_t BitPropVariant::getUInt8() const {
    switch ( vt ) {
        case VT_UI1:
            return bVal;
        default:
            throw BitException( "BitPropVariant is not a 8-bits unsigned integer" );
    }
}

uint16_t BitPropVariant::getUInt16() const {
    switch ( vt ) {
        case VT_UI1:
            return bVal;
        case VT_UI2:
            return uiVal;
        default:
            throw BitException( "BitPropVariant is not a 16-bits unsigned integer" );
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
            throw BitException( "BitPropVariant is not a 32-bits unsigned integer" );
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
            throw BitException( "BitPropVariant is not a 64-bits unsigned integer" );
    }
}

int8_t BitPropVariant::getInt8() const {
    switch ( vt ) {
        case VT_I1:
            return cVal;
        default:
            throw BitException( "BitPropVariant is not a 8-bits integer" );
    }
}

int16_t BitPropVariant::getInt16() const {
    switch ( vt ) {
        case VT_I1:
            return cVal;
        case VT_I2:
            return iVal;
        default:
            throw BitException( "BitPropVariant is not a 16-bits integer" );
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
            throw BitException( "BitPropVariant is not a 32-bits integer" );
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
            throw BitException( "BitPropVariant is not a 64-bits integer" );
    }
}

FILETIME BitPropVariant::getFiletime() const {
    if ( vt != VT_FILETIME ) {
        throw BitException( "BitPropVariant is not a FILETIME" );
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

bool BitPropVariant::isBool() const {
    return vt == VT_BOOL;
}

bool BitPropVariant::isString() const {
    return vt == VT_BSTR;
}

bool BitPropVariant::isUInt8() const {
    return vt == VT_UI1;
}

bool BitPropVariant::isUInt16() const {
    return vt == VT_UI2 || vt == VT_UI1;
}

bool BitPropVariant::isUInt32() const {
    return vt == VT_UI4 || vt == VT_UINT || vt == VT_UI2 || vt == VT_UI1;
}

bool BitPropVariant::isUInt64() const {
    return vt == VT_UI8 || vt == VT_UI4 || vt == VT_UINT || vt == VT_UI2 || vt == VT_UI1;
}

bool BitPropVariant::isInt8() const {
    return vt == VT_I1;
}

bool BitPropVariant::isInt16() const {
    return vt == VT_I2 || vt == VT_I1;
}

bool BitPropVariant::isInt32() const {
    return vt == VT_I4 || vt == VT_INT || vt == VT_I2 || vt == VT_I1;
}

bool BitPropVariant::isInt64() const {
    return vt == VT_I8 || vt == VT_I4 || vt == VT_INT || vt == VT_I2 || vt == VT_I1;
}

bool BitPropVariant::isFiletime() const {
    return vt == VT_FILETIME;
}

BitPropVariantType BitPropVariant::type() const {
    return lookupType( vt );
}

void BitPropVariant::clear() {
    if ( vt == VT_EMPTY ) {
        return;
    }
    internalClear();
    vt = VT_EMPTY;
}

void BitPropVariant::internalClear() {
    if ( vt == VT_BSTR && bstrVal != nullptr ) {
        ::SysFreeString( bstrVal ); //this was a string: since it is not needed anymore, we must free it!
        bstrVal = nullptr;
    }
    wReserved1 = 0;
    wReserved2 = 0;
    wReserved3 = 0;
    uhVal.QuadPart = 0;
}

bool bit7z::operator!=( const BitPropVariant& a, const BitPropVariant& b ) {
    return !( a == b );
}

bool bit7z::operator==( const BitPropVariant& a, const BitPropVariant& b ) {
    if ( a.vt != b.vt ) {
        return false;
    }
    switch ( a.vt ) { //a.vt == b.vt
        case VT_EMPTY:
            return true;
        case VT_BOOL:
            return a.boolVal == b.boolVal;
        case VT_BSTR:
            return wcscmp( a.bstrVal, b.bstrVal ) == 0;
        case VT_UI1:
            return a.bVal == b.bVal;
        case VT_UI2:
            return a.uiVal == b.uiVal;
        case VT_UINT:
            return a.uintVal == b.uintVal;
        case VT_UI4:
            return a.ulVal == b.ulVal;
        case VT_UI8:
            return a.uhVal.QuadPart == b.uhVal.QuadPart;
        case VT_I1:
            return a.cVal == b.cVal;
        case VT_I2:
            return a.iVal == b.iVal;
        case VT_INT:
            return a.intVal == b.intVal;
        case VT_I4:
            return a.lVal == b.lVal;
        case VT_I8:
            return a.hVal.QuadPart == b.hVal.QuadPart;
        case VT_FILETIME:
            return CompareFileTime( &a.filetime, &b.filetime ) == 0;
        default:
            return false;
    }
}
