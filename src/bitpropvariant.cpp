// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

/*
 * bit7z - A C++ static library to interface with the 7-zip shared libraries.
 * Copyright (c) 2014-2022 Riccardo Ostani - All Rights Reserved.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "bitpropvariant.hpp"

#include "bitexception.hpp"
#include "biterror.hpp"
#include "internal/dateutil.hpp"

#if defined(BIT7Z_USE_NATIVE_STRING) && defined(_WIN32) // Windows
#define BSTR_TO_TSTRING( bstr ) std::wstring( bstr, ::SysStringLen( bstr ) )
#else
#include "internal/util.hpp"
#define BSTR_TO_TSTRING( bstr ) bit7z::narrow( bstr, SysStringLen( bstr ) )
#endif

constexpr auto kCannotAllocateString = "Could not allocate memory for BitPropVariant string";

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
            return BitPropVariantType::FileTime;
        default:
            /* This is unlikely to happen since properties types used in archives
             * are the ones supported by PropertyType enum class.*/
            throw BitException( "Property type not supported", std::make_error_code( std::errc::invalid_argument ) );
    }
}

namespace bit7z { // Note: Clang doesn't find the operator if it is not inside the namespace.
/* Needed for comparing FILETIME objects in BitPropVariant */
inline bool operator==( const FILETIME& ft1, const FILETIME& ft2 ) noexcept {
#ifdef _WIN32
    return CompareFileTime( &ft1, &ft2 ) == 0;
#else
    return ft1.dwHighDateTime == ft2.dwHighDateTime && ft1.dwLowDateTime == ft2.dwLowDateTime;
#endif
}
}

BitPropVariant::BitPropVariant() : PROPVARIANT() {
    /* As in CPropVariant default constructor (Note: it seems that the default vt value is VT_NULL)*/
    vt = VT_EMPTY;
    wReserved1 = 0;
    bstrVal = nullptr;
}

BitPropVariant::BitPropVariant( const BitPropVariant& other ) : PROPVARIANT( other ) {
    if ( vt == VT_BSTR ) { //until now, we've copied only the pointer to the string, hence we need a deep copy!
        bstrVal = SysAllocStringByteLen( reinterpret_cast< LPCSTR >( other.bstrVal ),
                                         SysStringByteLen( other.bstrVal ) );
        if ( bstrVal == nullptr ) {
            throw BitException( kCannotAllocateString, std::make_error_code( std::errc::not_enough_memory ) );
        }
    }
}

BitPropVariant::BitPropVariant( BitPropVariant&& other ) noexcept: PROPVARIANT( other ) {
    if ( vt == VT_BSTR ) {
        /* this and other share the pointer to the same string, but now the string belongs to this!
         * Hence, if we set the other.bstrVal to nullptr, we prevent the bstrVal from being destroyed when
         * the other object is deleted! */
        other.bstrVal = nullptr;
    }
}

BitPropVariant::BitPropVariant( bool value ) noexcept: PROPVARIANT() {
    vt = VT_BOOL;
    wReserved1 = 0;
    boolVal = ( value ? VARIANT_TRUE : VARIANT_FALSE );
}

BitPropVariant::BitPropVariant( const wchar_t* value ) : PROPVARIANT() {
    vt = VT_BSTR;
    wReserved1 = 0;
    if ( value != nullptr ) {
        bstrVal = ::SysAllocString( value );
        if ( bstrVal == nullptr ) {
            throw BitException( kCannotAllocateString, std::make_error_code( std::errc::not_enough_memory ) );
        }
    } else {
        bstrVal = nullptr;
    }
}

BitPropVariant::BitPropVariant( const std::wstring& value ) : PROPVARIANT() {
    vt = VT_BSTR;
    wReserved1 = 0;
    bstrVal = ::SysAllocStringLen( value.c_str(), static_cast< unsigned int >( value.size() ) );
    if ( bstrVal == nullptr ) {
        throw BitException( kCannotAllocateString, std::make_error_code( std::errc::not_enough_memory ) );
    }
}

BitPropVariant::BitPropVariant( uint8_t value ) noexcept: PROPVARIANT() {
    vt = VT_UI1;
    wReserved1 = 0;
    bVal = value;
}

BitPropVariant::BitPropVariant( uint16_t value ) noexcept: PROPVARIANT() {
    vt = VT_UI2;
    wReserved1 = 0;
    uiVal = value;
}

BitPropVariant::BitPropVariant( uint32_t value ) noexcept: PROPVARIANT() {
    vt = VT_UI4;
    wReserved1 = 0;
    ulVal = value;
}

BitPropVariant::BitPropVariant( uint64_t value ) noexcept: PROPVARIANT() {
    vt = VT_UI8;
    wReserved1 = 0;
    uhVal.QuadPart = value;
}

BitPropVariant::BitPropVariant( int8_t value ) noexcept: PROPVARIANT() {
    vt = VT_I1;
    wReserved1 = 0;
    cVal = value;
}

BitPropVariant::BitPropVariant( int16_t value ) noexcept: PROPVARIANT() {
    vt = VT_I2;
    wReserved1 = 0;
    iVal = value;
}

BitPropVariant::BitPropVariant( int32_t value ) noexcept: PROPVARIANT() {
    vt = VT_I4;
    wReserved1 = 0;
    lVal = value;
}

BitPropVariant::BitPropVariant( int64_t value ) noexcept: PROPVARIANT() {
    vt = VT_I8;
    wReserved1 = 0;
    hVal.QuadPart = value;
}

BitPropVariant::BitPropVariant( const FILETIME& value ) noexcept: PROPVARIANT() {
    vt = VT_FILETIME;
    wReserved1 = 0;
    filetime = value;
}

BitPropVariant::~BitPropVariant() {
    internalClear();
}

BitPropVariant& BitPropVariant::operator=( const BitPropVariant& other ) {
    BitPropVariant tmp( other ); //copy construct a tmp variable
    *this = std::move( tmp ); //move assign to this
    return *this;
}

BitPropVariant& BitPropVariant::operator=( BitPropVariant&& other ) noexcept {
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
            default: //type not supported
                break;
        }
    }
    return *this;
}

bool BitPropVariant::getBool() const {
    if ( vt != VT_BOOL ) {
        throw BitException( "BitPropVariant is not a bool", make_error_code( BitError::RequestedWrongVariantType ) );
    }
    return boolVal != VARIANT_FALSE; //simply returning boolVal should work but this prevents some compiler warnings.
}

tstring BitPropVariant::getString() const {
    if ( vt != VT_BSTR ) {
        throw BitException( "BitPropVariant is not a string", make_error_code( BitError::RequestedWrongVariantType ) );
    }
    //Note: a nullptr BSTR is semantically equivalent to an empty string!
    return bstrVal == nullptr ? tstring{} : BSTR_TO_TSTRING( bstrVal );
}

uint8_t BitPropVariant::getUInt8() const {
    switch ( vt ) {
        case VT_UI1:
            return bVal;
        default: // not an 8-bits unsigned integer.
            throw BitException( "BitPropVariant is not an 8-bits unsigned integer",
                                make_error_code( BitError::RequestedWrongVariantType ) );
    }
}

uint16_t BitPropVariant::getUInt16() const {
    switch ( vt ) {
        case VT_UI1:
            return bVal;
        case VT_UI2:
            return uiVal;
        default: // not a 16-bits unsigned integer.
            throw BitException( "BitPropVariant is not a 16-bits unsigned integer",
                                make_error_code( BitError::RequestedWrongVariantType ) );
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
        default: // not a 32-bits unsigned integer.
            throw BitException( "BitPropVariant is not a 32-bits unsigned integer",
                                make_error_code( BitError::RequestedWrongVariantType ) );
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
        default: // not a 64-bits unsigned integer.
            throw BitException( "BitPropVariant is not a 64-bits unsigned integer",
                                make_error_code( BitError::RequestedWrongVariantType ) );
    }
}

int8_t BitPropVariant::getInt8() const {
    switch ( vt ) {
        case VT_I1:
            return cVal;
        default: // not an 8-bits integer.
            throw BitException( "BitPropVariant is not an 8-bits integer",
                                make_error_code( BitError::RequestedWrongVariantType ) );
    }
}

int16_t BitPropVariant::getInt16() const {
    switch ( vt ) {
        case VT_I1:
            return cVal;
        case VT_I2:
            return iVal;
        default: // not a 16-bits integer.
            throw BitException( "BitPropVariant is not a 16-bits integer",
                                make_error_code( BitError::RequestedWrongVariantType ) );
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
        default: // not a 32-bits integer.
            throw BitException( "BitPropVariant is not a 32-bits integer",
                                make_error_code( BitError::RequestedWrongVariantType ) );
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
        default: // not a 64-bits integer.
            throw BitException( "BitPropVariant is not a 64-bits integer",
                                make_error_code( BitError::RequestedWrongVariantType ) );
    }
}

FILETIME BitPropVariant::getFileTime() const {
    if ( vt != VT_FILETIME ) {
        throw BitException( "BitPropVariant is not a FILETIME",
                            make_error_code( BitError::RequestedWrongVariantType ) );
    }
    return filetime;
}

bit7z::time_type BitPropVariant::getTimePoint() const {
    FILETIME ft = getFileTime();
    return FILETIME_to_time_type( ft );
}

tstring BitPropVariant::toString() const {
    switch ( vt ) {
        case VT_BOOL:
            return boolVal == VARIANT_TRUE ? BIT7Z_STRING( "true" ) : BIT7Z_STRING( "false" );
        case VT_BSTR:
            return BSTR_TO_TSTRING( bstrVal );
        case VT_UI1:
            return to_tstring( bVal );
        case VT_UI2:
            return to_tstring( uiVal );
        case VT_UINT:
            return to_tstring( uintVal );
        case VT_UI4:
            return to_tstring( ulVal );
        case VT_UI8:
            return to_tstring( uhVal.QuadPart );
        case VT_I1:
            return to_tstring( cVal );
        case VT_I2:
            return to_tstring( iVal );
        case VT_INT:
            return to_tstring( intVal );
        case VT_I4:
            return to_tstring( lVal );
        case VT_I8:
            return to_tstring( hVal.QuadPart );
        case VT_FILETIME:
            return to_tstring( filetime.dwHighDateTime ) + BIT7Z_STRING( ", " ) + to_tstring( filetime.dwLowDateTime );
        case VT_EMPTY:
            return tstring{};
        default: //type not supported
            throw BitException( "BitPropVariant type code " + std::to_string( vt ) + " not supported.",
                                make_error_code( BitError::UnsupportedOperation ) );
    }
}

bool BitPropVariant::isEmpty() const noexcept {
    return vt == VT_EMPTY;
}

bool BitPropVariant::isBool() const noexcept {
    return vt == VT_BOOL;
}

bool BitPropVariant::isString() const noexcept {
    return vt == VT_BSTR;
}

bool BitPropVariant::isUInt8() const noexcept {
    return vt == VT_UI1;
}

bool BitPropVariant::isUInt16() const noexcept {
    return vt == VT_UI2 || vt == VT_UI1;
}

bool BitPropVariant::isUInt32() const noexcept {
    return vt == VT_UI4 || vt == VT_UINT || vt == VT_UI2 || vt == VT_UI1;
}

bool BitPropVariant::isUInt64() const noexcept {
    return vt == VT_UI8 || vt == VT_UI4 || vt == VT_UINT || vt == VT_UI2 || vt == VT_UI1;
}

bool BitPropVariant::isInt8() const noexcept {
    return vt == VT_I1;
}

bool BitPropVariant::isInt16() const noexcept {
    return vt == VT_I2 || vt == VT_I1;
}

bool BitPropVariant::isInt32() const noexcept {
    return vt == VT_I4 || vt == VT_INT || vt == VT_I2 || vt == VT_I1;
}

bool BitPropVariant::isInt64() const noexcept {
    return vt == VT_I8 || vt == VT_I4 || vt == VT_INT || vt == VT_I2 || vt == VT_I1;
}

bool BitPropVariant::isFileTime() const noexcept {
    return vt == VT_FILETIME;
}

BitPropVariantType BitPropVariant::type() const {
    return lookupType( vt );
}

void BitPropVariant::clear() noexcept {
    if ( vt == VT_EMPTY ) {
        return;
    }
    internalClear();
    vt = VT_EMPTY;
}

void BitPropVariant::internalClear() noexcept {
    if ( vt == VT_BSTR && bstrVal != nullptr ) {
        ::SysFreeString( bstrVal ); //this was a string: since it is not needed anymore, we must free it!
        bstrVal = nullptr;
    }
    wReserved1 = 0;
    wReserved2 = 0;
    wReserved3 = 0;
    uhVal.QuadPart = 0;
}

bool bit7z::operator!=( const BitPropVariant& a, const BitPropVariant& b ) noexcept {
    return !( a == b );
}

bool bit7z::operator==( const BitPropVariant& a, const BitPropVariant& b ) noexcept {
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
            return a.filetime == b.filetime;
        default:
            return false;
    }
}
