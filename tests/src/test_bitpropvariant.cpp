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
#ifdef _WIN32
#define NOMINMAX
#endif

#include <catch2/catch.hpp>

#include <bit7z/bitpropvariant.hpp>
#ifndef _WIN32
#include <internal/windows.hpp> // for VT_ enum constants
#endif

#include <map>
#include <limits>
#include <sstream>
#include <cstring>

//-V::530,2008,2563,2608,3555 (Suppressing warnings in PVS-Studio)

#if !defined(BIT7Z_USE_NATIVE_STRING) && defined(_WIN32)

auto convert_string_to_bstr( const std::string& str ) -> BSTR {
    const int wideLength = ::MultiByteToWideChar( CP_ACP, 0 /* no flags */,
                                                  str.data(), static_cast<int>(str.size()),
                                                  nullptr, 0 );

    BSTR wideString = ::SysAllocStringLen( nullptr, wideLength );
    ::MultiByteToWideChar( CP_ACP, 0 /* no flags */,
                           str.data(), static_cast<int>(str.size()),
                           wideString, wideLength );
    return wideString;
}

#endif

using namespace std;
using namespace bit7z;

constexpr auto kTestWideString = L"abcdefghijklmnopqrstuvwxyz0123456789";
constexpr auto kTestTstring = BIT7Z_STRING( "abcdefghijklmnopqrstuvwxyz0123456789" );
constexpr auto kTestNativeString = BIT7Z_NATIVE_STRING( "abcdefghijklmnopqrstuvwxyz0123456789" );

constexpr auto kTestInputEncoding = L"\u30a6\u30a9\u30eb\u30b0\u30e9\u30a4\u30e2\u30f3"; // ウォルグライモン
constexpr auto kTestOutputEncoding = BIT7Z_STRING( "\u30a6\u30a9\u30eb\u30b0\u30e9\u30a4\u30e2\u30f3" ); // ウォルグライモン

void check_variant_type( const BitPropVariant& propVariant, BitPropVariantType type ) {
    REQUIRE( propVariant.type() == type );
    REQUIRE( propVariant.isEmpty() == ( type == BitPropVariantType::Empty ) );
    REQUIRE( propVariant.isBool() == ( type == BitPropVariantType::Bool ) );
    REQUIRE( propVariant.isString() == ( type == BitPropVariantType::String ) );
    REQUIRE( propVariant.isUInt8() == ( type == BitPropVariantType::UInt8 ||
                                         type == BitPropVariantType::UInt16 ||
                                         type == BitPropVariantType::UInt32 ||
                                         type == BitPropVariantType::UInt64 ) );
    REQUIRE( propVariant.isUInt16() == ( type == BitPropVariantType::UInt16 ||
                                          type == BitPropVariantType::UInt32 ||
                                          type == BitPropVariantType::UInt64 ) );
    REQUIRE( propVariant.isUInt32() == ( type == BitPropVariantType::UInt32 ||
                                          type == BitPropVariantType::UInt64 ) );
    REQUIRE( propVariant.isUInt64() == ( type == BitPropVariantType::UInt64 ) );
    REQUIRE( propVariant.isInt8() == ( type == BitPropVariantType::Int8 ||
                                        type == BitPropVariantType::Int16 ||
                                        type == BitPropVariantType::Int32 ||
                                        type == BitPropVariantType::Int64 ) );
    REQUIRE( propVariant.isInt16() == ( type == BitPropVariantType::Int16 ||
                                         type == BitPropVariantType::Int32 ||
                                         type == BitPropVariantType::Int64 ) );
    REQUIRE( propVariant.isInt32() == ( type == BitPropVariantType::Int32 ||
                                         type == BitPropVariantType::Int64 ) );
    REQUIRE( propVariant.isInt64() == ( type == BitPropVariantType::Int64 ) );
    REQUIRE( propVariant.isFileTime() == ( type == BitPropVariantType::FileTime ) );

    if ( type != BitPropVariantType::Bool ) {
        REQUIRE_THROWS( propVariant.getBool() );
    }
    if ( type != BitPropVariantType::String ) {
        REQUIRE_THROWS( propVariant.getString() );
        REQUIRE_THROWS( propVariant.getNativeString() );
    }

    if ( type != BitPropVariantType::UInt64 ) {
        REQUIRE_THROWS( propVariant.getUInt64() );
        if ( type != BitPropVariantType::UInt32 ) {
            REQUIRE_THROWS( propVariant.getUInt32() );
            if ( type != BitPropVariantType::UInt16 ) {
                REQUIRE_THROWS( propVariant.getUInt16() );
                if ( type != BitPropVariantType::UInt8 ) {
                    REQUIRE_THROWS( propVariant.getUInt8() );
                }
            }
        }
    }

    if ( type != BitPropVariantType::Int64 ) {
        REQUIRE_THROWS( propVariant.getInt64() );
        if ( type != BitPropVariantType::Int32 ) {
            REQUIRE_THROWS( propVariant.getInt32() );
            if ( type != BitPropVariantType::Int16 ) {
                REQUIRE_THROWS( propVariant.getInt16() );
                if ( type != BitPropVariantType::Int8 ) {
                    REQUIRE_THROWS( propVariant.getInt8() );
                }
            }
        }
    }

    if ( type != BitPropVariantType::FileTime ) {
        REQUIRE_THROWS( propVariant.getFileTime() );
    }
}

TEST_CASE( "BitPropVariant: Empty variant", "[BitPropVariant][empty]" ) {
    BitPropVariant propVariant;
    REQUIRE( propVariant.vt == VT_EMPTY );
    REQUIRE( propVariant.toString().empty() );

    check_variant_type( propVariant, BitPropVariantType::Empty );
    REQUIRE_NOTHROW( propVariant.clear() );
    REQUIRE( propVariant.isEmpty() ); // still empty after clear
}

TEST_CASE( "BitPropVariant: Boolean variant", "[BitPropVariant][boolean]" ) {
    BitPropVariant propVariant;

    SECTION( "Setting to true" ) {
        SECTION( "Using the constructor" ) {
            propVariant = BitPropVariant( true );
        }

        SECTION( "Using manual assignment" ) {
            propVariant.vt = VT_BOOL;
            propVariant.boolVal = VARIANT_TRUE;
        }

        SECTION( "Using assignment" ) {
            propVariant = true;
        }

        SECTION( "Using double assignment" ) {
            propVariant = false;
            REQUIRE( !propVariant.getBool() );
            propVariant = true;
        }

        SECTION( "Using double assignment (different type)" ) {
            propVariant = L"hello world!";
            (void)propVariant;
            propVariant = true;
        }

        REQUIRE( propVariant.getBool() );
        REQUIRE( propVariant.toString() == BIT7Z_STRING( "true" ) );
    }

    SECTION( "Setting to false" ) {
        SECTION( "Using the constructor" ) {
            propVariant = BitPropVariant( false );
        }

        SECTION( "Using manual assignment" ) {
            propVariant.vt = VT_BOOL;
            propVariant.boolVal = VARIANT_FALSE;
        }

        SECTION( "Using assignment" ) {
            propVariant = false;
        }

        SECTION( "Using double assignment" ) {
            propVariant = true;
            REQUIRE( propVariant.getBool() );
            propVariant = false;
        }

        SECTION( "Using double assignment (different type)" ) {
            propVariant = L"hello world!";
            (void)propVariant;
            propVariant = false;
        }

        REQUIRE( !propVariant.getBool() );
        REQUIRE( propVariant.toString() == BIT7Z_STRING( "false" ) );
    }

    check_variant_type( propVariant, BitPropVariantType::Bool );
    REQUIRE_NOTHROW( propVariant.clear() );
    REQUIRE( propVariant.isEmpty() );
}

TEST_CASE( "BitPropVariant: String variant", "[BitPropVariant][string]" ) {
    BitPropVariant propVariant;

    SECTION( "Initializing with an empty string" ) {
        SECTION( "Using the constructor (with empty wstring)" ) {
            propVariant = BitPropVariant( wstring() );
        }

        SECTION( "Using the constructor (with empty C wide string)" ) {
            propVariant = BitPropVariant( L"" );
        }

        SECTION( "Using the constructor (with nullptr C wide string)" ) {
            propVariant = BitPropVariant( nullptr );
        }

        SECTION( "Manually setting to nullptr" ) {
            propVariant.vt = VT_BSTR;
            propVariant.bstrVal = nullptr; //semantically equivalent to empty string
        }

        SECTION( "Assigning an empty wstring" ) {
            propVariant = wstring();
        }

        SECTION( "Assigning an empty C wide string" ) {
            propVariant = L"";
        }

        SECTION( "Assigning a nullptr C wide string" ) { //equivalent to empty bstr
            propVariant = nullptr; //nullptr const wchar_t* assignment
        }

        SECTION( "Double assignment" ) {
            propVariant = L"lorem ipsum";
            REQUIRE( !propVariant.getString().empty() );
            propVariant = L"";
        }

        REQUIRE( propVariant.getString().empty() );
        REQUIRE( propVariant.getNativeString().empty() );
        REQUIRE( propVariant.toString().empty() );
    }

    SECTION( "Initializing with a non-empty string" ) {
        SECTION( "Using the constructor (with a wstring)" ) {
            propVariant = BitPropVariant( std::wstring( kTestWideString ) );
        }

        SECTION( "Using the constructor (with a C string)" ) {
            propVariant = BitPropVariant( kTestWideString );
        }

#ifdef _WIN32
        SECTION( "Manually setting it" ) {
            propVariant.vt = VT_BSTR;
#ifdef BIT7Z_USE_NATIVE_STRING
            // Note: flawfinder complains about using wcslen on a possibly non-null terminating string,
            // but kTestTstring is guaranteed to be a null-terminated string!
            const auto test_tstring_size = static_cast< UINT >( wcslen( kTestTstring ) ); // flawfinder: ignore
            propVariant.bstrVal = SysAllocStringLen( kTestTstring, test_tstring_size );
#else
            propVariant.bstrVal = convert_string_to_bstr( kTestTstring );
#endif
        }
#endif

        SECTION( "Assigning a wstring" ) {
            propVariant = std::wstring( kTestWideString );
        }

        SECTION( "Assigning a C string" ) {
            propVariant = kTestWideString;
        }

        SECTION( "Double assignment" ) {
            propVariant = L"lorem ipsum";
            REQUIRE( propVariant.getString() == BIT7Z_STRING( "lorem ipsum" ) );
            propVariant = kTestWideString;
        }

        REQUIRE( propVariant.bstrVal != nullptr );
        REQUIRE( propVariant.getString() == kTestTstring );
        REQUIRE( propVariant.getNativeString() == kTestNativeString );
        REQUIRE( propVariant.toString() == kTestTstring );
    }

#ifndef BIT7Z_USE_SYSTEM_CODEPAGE
    SECTION( "Initializing with non-ASCII encoded string" ) {
        SECTION( "String literal constructor" ) {
            propVariant = BitPropVariant{ kTestInputEncoding };
        }

        SECTION( "Wide string constructor" ) {
            propVariant = BitPropVariant{ std::wstring( kTestInputEncoding ) };
        }

        auto encodedString = propVariant.getString();
        REQUIRE( encodedString == kTestOutputEncoding );
    }
#endif

    check_variant_type( propVariant, BitPropVariantType::String );
    REQUIRE_NOTHROW( propVariant.clear() );
    REQUIRE( propVariant.isEmpty() );
    REQUIRE( propVariant.bstrVal == nullptr );
}

template< typename T, size_t S >
using is_unsigned_with_size = std::integral_constant< bool, std::is_unsigned< T >::value && S == sizeof( T ) >;

template< typename T, size_t S >
using is_signed_with_size = std::integral_constant< bool, std::is_signed< T >::value && S == sizeof( T ) >;

template< typename T, typename std::enable_if< is_unsigned_with_size< T, 1 >::value >::type* = nullptr >
void manually_set_variant( BitPropVariant& prop, T value ) {
    prop.vt = VT_UI1;
    prop.bVal = value;
}

template< typename T, typename std::enable_if< is_unsigned_with_size< T, 2 >::value >::type* = nullptr >
void manually_set_variant( BitPropVariant& prop, T value ) {
    prop.vt = VT_UI2;
    prop.uiVal = value;
}

template< typename T, typename std::enable_if< is_unsigned_with_size< T, 4 >::value >::type* = nullptr >
void manually_set_variant( BitPropVariant& prop, T value ) {
    prop.vt = VT_UI4;
    prop.ulVal = value;
}

template< typename T, typename std::enable_if< is_unsigned_with_size< T, 8 >::value >::type* = nullptr >
void manually_set_variant( BitPropVariant& prop, T value ) {
    prop.vt = VT_UI8;
    prop.uhVal.QuadPart = value;
}

template< typename T, typename std::enable_if< is_signed_with_size< T, 1 >::value >::type* = nullptr >
void manually_set_variant( BitPropVariant& prop, T value ) {
    prop.vt = VT_I1;
    prop.cVal = value;
}

template< typename T, typename std::enable_if< is_signed_with_size< T, 2 >::value >::type* = nullptr >
void manually_set_variant( BitPropVariant& prop, T value ) {
    prop.vt = VT_I2;
    prop.iVal = value;
}

template< typename T, typename std::enable_if< is_signed_with_size< T, 4 >::value >::type* = nullptr >
void manually_set_variant( BitPropVariant& prop, T value ) {
    prop.vt = VT_I4;
    prop.lVal = value;
}

template< typename T, typename std::enable_if< is_signed_with_size< T, 8 >::value >::type* = nullptr >
void manually_set_variant( BitPropVariant& prop, T value ) {
    prop.vt = VT_I8;
    prop.hVal.QuadPart = value;
}

template< typename T >
auto variant_type() -> BitPropVariantType {
    if ( is_unsigned_with_size< T, 1 >::value ) {
        return BitPropVariantType::UInt8;
    }
    if ( is_unsigned_with_size< T, 2 >::value ) {
        return BitPropVariantType::UInt16;
    }
    if ( is_unsigned_with_size< T, 4 >::value ) {
        return BitPropVariantType::UInt32;
    }
    if ( is_unsigned_with_size< T, 8 >::value ) {
        return BitPropVariantType::UInt64;
    }
    if ( is_signed_with_size< T, 1 >::value ) {
        return BitPropVariantType::Int8;
    }
    if ( is_signed_with_size< T, 2 >::value ) {
        return BitPropVariantType::Int16;
    }
    if ( is_signed_with_size< T, 4 >::value ) {
        return BitPropVariantType::Int32;
    }
    if ( is_signed_with_size< T, 8 >::value ) {
        return BitPropVariantType::Int64;
    }
    // Should not happen in the tests in which we use this function!
    return BitPropVariantType::Empty;
}

template< typename T >
auto get_value( const BitPropVariant& variant ) -> T {
    if ( is_unsigned_with_size< T, 1 >::value ) {
        return variant.getUInt8();
    }
    if ( is_unsigned_with_size< T, 2 >::value ) {
        return variant.getUInt16();
    }
    if ( is_unsigned_with_size< T, 4 >::value ) {
        return variant.getUInt32();
    }
    if ( is_unsigned_with_size< T, 8 >::value ) {
        return variant.getUInt64();
    }
    if ( is_signed_with_size< T, 1 >::value ) {
        return variant.getInt8();
    }
    if ( is_signed_with_size< T, 2 >::value ) {
        return variant.getInt16();
    }
    if ( is_signed_with_size< T, 4 >::value ) {
        return variant.getInt32();
    }
    if ( is_signed_with_size< T, 8 >::value ) {
        return variant.getInt64();
    }
    return {};
}

// BitPropVariant's toString uses std::to_string so here we use another way to convert integers to string
auto uint_to_tstring( uint64_t value ) -> tstring {
#if defined( _WIN32 ) && defined( BIT7Z_USE_NATIVE_STRING )
    std::wstringstream stream;
#else
    std::stringstream stream;
#endif
    stream << std::dec << uint64_t{ value };
    return stream.str();
}

TEMPLATE_TEST_CASE( "BitPropVariant: Unsigned integer variant", "[BitPropVariant][unsigned]",
                    uint8_t, uint16_t, uint32_t, uint64_t ) {
    BitPropVariant propVariant;

    TestType value = GENERATE( 0, 42, std::numeric_limits< TestType >::max() );

    SECTION( "Using the unsigned integer constructor" ) {
        propVariant = BitPropVariant( value );
    }

    SECTION( "Manually setting the unsigned integer value" ) {
        manually_set_variant( propVariant, value );
    }

    SECTION( "Assigning the unsigned integer value" ) {
        propVariant = value;
    }

    SECTION( "Double assignment" ) {
        propVariant = static_cast< uint64_t >( 84ull ); // NOLINT(*-magic-numbers)
        REQUIRE( propVariant.getUInt64() == 84ull );
        propVariant = value;
    }

    REQUIRE( get_value< TestType >( propVariant ) == value );
    REQUIRE( propVariant.toString() == uint_to_tstring( value ) );

    REQUIRE( propVariant.type() == variant_type< TestType >() );
    REQUIRE( !propVariant.isEmpty() );
    REQUIRE( !propVariant.isBool() );
    REQUIRE( !propVariant.isString() );

    // The variant is an UInt8 only if the TestType size is 8 bits.
    REQUIRE( propVariant.isUInt8() == ( sizeof( TestType ) == 1 ) );

    // The variant is an UInt16 only if the TestType size is at most 16 bits.
    REQUIRE( propVariant.isUInt16() == ( sizeof( TestType ) <= 2 ) );

    // The variant is an UInt32 only if the TestType size is at most 32 bits.
    REQUIRE( propVariant.isUInt32() == ( sizeof( TestType ) <= 4 ) ); //-V112

    // All unsigned types can be contained in a UInt64.
    REQUIRE( propVariant.isUInt64() );
    REQUIRE( !propVariant.isInt8() );
    REQUIRE( !propVariant.isInt16() );
    REQUIRE( !propVariant.isInt32() );
    REQUIRE( !propVariant.isInt64() );
    REQUIRE( !propVariant.isFileTime() );
    REQUIRE_THROWS( propVariant.getString() );
    REQUIRE_THROWS( propVariant.getNativeString() );
    REQUIRE_THROWS( propVariant.getBool() );
    REQUIRE_THROWS( propVariant.getInt8() );
    REQUIRE_THROWS( propVariant.getInt16() );
    REQUIRE_THROWS( propVariant.getInt32() );
    REQUIRE_THROWS( propVariant.getInt64() );
    REQUIRE_THROWS( propVariant.getFileTime() );
    if ( sizeof( TestType ) == 1 ) {
        REQUIRE( propVariant.getUInt8() == propVariant.getUInt16() );
        REQUIRE( propVariant.getUInt8() == propVariant.getUInt32() );
        REQUIRE( propVariant.getUInt8() == propVariant.getUInt64() );
    } else {
        REQUIRE_THROWS( propVariant.getUInt8() );
    }
    if ( sizeof( TestType ) <= 2 ) {
        REQUIRE( propVariant.getUInt16() == propVariant.getUInt32() );
        REQUIRE( propVariant.getUInt16() == propVariant.getUInt64() );
    } else {
        REQUIRE_THROWS( propVariant.getUInt16() );
    }
    if ( sizeof( TestType ) <= 4 ) { //-V112
        REQUIRE( propVariant.getUInt32() == propVariant.getUInt64() );
    } else {
        REQUIRE_THROWS( propVariant.getUInt32() );
    }
    REQUIRE_NOTHROW( propVariant.clear() );
    REQUIRE( propVariant.isEmpty() );
}

auto int_to_tstring( int64_t value ) -> tstring {
#if defined( _WIN32 ) && defined( BIT7Z_USE_NATIVE_STRING )
    std::wstringstream stream;
#else
    std::stringstream stream;
#endif
    stream << std::dec << int64_t{ value };
    return stream.str();
}

TEMPLATE_TEST_CASE( "BitPropVariant: Integer variant", "[BitPropVariant][signed]",
                    int8_t, int16_t, int32_t, int64_t ) {
    BitPropVariant propVariant;

    TestType value = GENERATE( std::numeric_limits< TestType >::min(),
                               -42,
                               0,
                               42,
                               std::numeric_limits< TestType >::max() );

    SECTION( "Using the signed integer constructor" ) {
        propVariant = BitPropVariant( value );
    }

    SECTION( "Manually setting the signed integer value" ) {
        manually_set_variant( propVariant, value );
    }

    SECTION( "Assigning the signed integer value" ) {
        propVariant = value;
    }

    SECTION( "Double assignment" ) {
        propVariant = static_cast< uint64_t >( 84ull ); // NOLINT(*-magic-numbers)
        REQUIRE( propVariant.getUInt64() == 84ull );
        propVariant = value;
    }

    REQUIRE( get_value< TestType >( propVariant ) == value );
    REQUIRE( propVariant.toString() == int_to_tstring( value ) );

    REQUIRE( propVariant.type() == variant_type< TestType >() );
    REQUIRE( !propVariant.isEmpty() );
    REQUIRE( !propVariant.isBool() );
    REQUIRE( !propVariant.isString() );

    // The variant is an Int8 only if the TestType size is 8 bits.
    REQUIRE( propVariant.isInt8() == ( sizeof( TestType ) == 1 ) );

    // The variant is an Int16 only if the TestType size is at most 16 bits.
    REQUIRE( propVariant.isInt16() == ( sizeof( TestType ) <= 2 ) );

    // The variant is an Int32 only if the TestType size is at most 32 bits.
    REQUIRE( propVariant.isInt32() == ( sizeof( TestType ) <= 4 ) ); //-V112

    // All signed types can be contained in an Int64.
    REQUIRE( propVariant.isInt64() );

    REQUIRE( !propVariant.isUInt8() );
    REQUIRE( !propVariant.isUInt16() );
    REQUIRE( !propVariant.isUInt32() );
    REQUIRE( !propVariant.isUInt64() );
    REQUIRE( !propVariant.isFileTime() );
    REQUIRE_THROWS( propVariant.getString() );
    REQUIRE_THROWS( propVariant.getNativeString() );
    REQUIRE_THROWS( propVariant.getBool() );
    REQUIRE_THROWS( propVariant.getUInt8() );
    REQUIRE_THROWS( propVariant.getUInt16() );
    REQUIRE_THROWS( propVariant.getUInt32() );
    REQUIRE_THROWS( propVariant.getUInt64() );
    REQUIRE_THROWS( propVariant.getFileTime() );
    if ( sizeof( TestType ) == 1 ) {
        REQUIRE( propVariant.getInt8() == propVariant.getInt16() );
        REQUIRE( propVariant.getInt8() == propVariant.getInt32() );
        REQUIRE( propVariant.getInt8() == propVariant.getInt64() );
    } else {
        REQUIRE_THROWS( propVariant.getInt8() );
    }
    if ( sizeof( TestType ) <= 2 ) {
        REQUIRE( propVariant.getInt16() == propVariant.getInt32() );
        REQUIRE( propVariant.getInt16() == propVariant.getInt64() );
    } else {
        REQUIRE_THROWS( propVariant.getInt16() );
    }
    if ( sizeof( TestType ) <= 4 ) { //-V112
        REQUIRE( propVariant.getInt32() == propVariant.getInt64() );
    } else {
        REQUIRE_THROWS( propVariant.getInt32() );
    }
    REQUIRE_NOTHROW( propVariant.clear() );
    REQUIRE( propVariant.isEmpty() );
}

TEST_CASE( "BitPropVariant: FILETIME variant", "[BitPropVariant][FILETIME]" ) {
    //NOLINTNEXTLINE(*-magic-numbers)
    FILETIME value{ 3017121792, 30269298 }; // 21 December 2012, 12:00
    BitPropVariant propVariant;

    SECTION( "Using the constructor" ) {
        propVariant = BitPropVariant( value );
    }

    SECTION( "Manually setting" ) {
        propVariant.vt = VT_FILETIME;
        propVariant.filetime = value;
    }

    SECTION( "Assignment" ) {
        propVariant = value;
    }

    SECTION( "Double assignment" ) {
        propVariant = static_cast< uint64_t >( 84ull ); // NOLINT(*-magic-numbers)
        REQUIRE( propVariant.getUInt64() == 84ull );
        propVariant = value;
    }

    auto result = propVariant.getFileTime();
    REQUIRE( std::memcmp( &result, &value, sizeof( FILETIME ) ) == 0 );

    check_variant_type( propVariant, BitPropVariantType::FileTime );
    REQUIRE_NOTHROW( propVariant.clear() );
    REQUIRE( propVariant.isEmpty() );
}

TEST_CASE( "BitPropVariant: Copying string variants", "[BitPropVariant][copy]" ) {
    const BitPropVariant propVariant{ std::wstring( kTestWideString ) };

    SECTION( "Copy constructor" ) {
        const BitPropVariant copyVar( propVariant ); //copy constructor
        REQUIRE( !copyVar.isEmpty() );
        REQUIRE( copyVar.vt == propVariant.vt );
        REQUIRE( copyVar.type() == propVariant.type() );
        REQUIRE( copyVar.bstrVal != propVariant.bstrVal );
        REQUIRE( wcscmp( copyVar.bstrVal, propVariant.bstrVal ) == 0 );
        REQUIRE( copyVar.getString() == propVariant.getString() );
        REQUIRE( copyVar.getNativeString() == propVariant.getNativeString() );
    }

    SECTION( "Copy assignment" ) {
        const BitPropVariant copyVar = propVariant;
        REQUIRE( !copyVar.isEmpty() );
        REQUIRE( copyVar.vt == propVariant.vt );
        REQUIRE( copyVar.type() == propVariant.type() );
        REQUIRE( copyVar.bstrVal != propVariant.bstrVal );
        REQUIRE( wcscmp( copyVar.bstrVal, propVariant.bstrVal ) == 0 );
        REQUIRE( copyVar.getString() == propVariant.getString() );
        REQUIRE( copyVar.getNativeString() == propVariant.getNativeString() );
    }
}

TEST_CASE( "BitPropVariant: Moving string variants", "[BitPropVariant][copy]" ) {
    BitPropVariant propVariant{ std::wstring( kTestWideString ) };

    SECTION( "Move constructor" ) {
        // The move may invalidate propvariant.bstrVal (make it nullptr), so we copy the pointer
        // and check the moved variant uses the same pointer!
        BSTR testBstrVal = propVariant.bstrVal;
        const BitPropVariant moveVar( std::move( propVariant ) );
        REQUIRE( !moveVar.isEmpty() );
        REQUIRE( moveVar.vt == VT_BSTR );
        REQUIRE( moveVar.bstrVal ==
                 testBstrVal ); //moveVar should point to the same BSTR object of the original propVariant!
        REQUIRE( moveVar.getString() == kTestTstring );
        REQUIRE( moveVar.getNativeString() == kTestNativeString );
    }

    SECTION( "Move assignment" ) {
        // The move may invalidate propvariant.bstrVal (make it nullptr), so we copy the pointer
        // and check if the moved variant uses the same pointer!
        BSTR testBstrVal = propVariant.bstrVal; // NOLINT(bugprone-use-after-move)
        const BitPropVariant moveVar = std::move( propVariant ); // cppcheck-suppress accessMoved
        REQUIRE( !moveVar.isEmpty() );
        REQUIRE( moveVar.vt == VT_BSTR );
        REQUIRE( moveVar.bstrVal ==
                 testBstrVal ); //moveVar should point to the same BSTR object of the original propVariant!
        REQUIRE( moveVar.getString() == kTestTstring );
        REQUIRE( moveVar.getNativeString() == kTestNativeString );
    }
}

TEST_CASE( "BitPropVariant: Equality operator", "[bitpropvariant][equality]" ) {
    BitPropVariant first;
    BitPropVariant second;
    REQUIRE( first == second );

    SECTION( "Comparing equal variants" ) {
        SECTION( "Same signed integers" ) {
            first = 42; // NOLINT(*-magic-numbers)
            second = 42; // NOLINT(*-magic-numbers)
        }

        SECTION( "Same unsigned integers" ) {
            first = 42u; // NOLINT(*-magic-numbers)
            second = 42u; // NOLINT(*-magic-numbers)
        }

        SECTION( "Same booleans (true)" ) {
            first = true;
            second = true;
        }

        SECTION( "Same booleans (true)" ) {
            first = false;
            second = false;
        }

        SECTION( "Same C strings" ) {
            first = L"hello world";
            second = L"hello world";
            REQUIRE( first.bstrVal != second.bstrVal );
        }

        SECTION( "Same std::wstrings" ) {
            first = std::wstring( kTestWideString );
            second = std::wstring( kTestWideString );
            REQUIRE( first.bstrVal != second.bstrVal );
        }

        REQUIRE( first == second );
    }

    SECTION( "Comparing different variants" ) {
        SECTION( "Different signed integers" ) {
            first = 42; // NOLINT(*-magic-numbers)
            second = 84; // NOLINT(*-magic-numbers)
        }

        SECTION( "Different unsigned integers" ) {
            first = 42u; // NOLINT(*-magic-numbers)
            second = 84u; // NOLINT(*-magic-numbers)
        }

        SECTION( "Integers with different signedness" ) {
            first = 42; // NOLINT(*-magic-numbers)
            second = 42u; // NOLINT(*-magic-numbers)
            //unsigned 42 (second) is different from first signed 42 (first)!
        }

        SECTION( "Different booleans (1)" ) {
            first = true;
            second = false;
        }

        SECTION( "Different booleans (2)" ) {
            first = false;
            second = true;
        }

        SECTION( "Integer and first string" ) {
            first = 42; // NOLINT(*-magic-numbers)
            second = L"ciao mondo";
        }

        SECTION( "Different C strings" ) {
            first = L"hello world";
            second = L"ciao mondo";
            REQUIRE( first.bstrVal != second.bstrVal );
        }

        SECTION( "Different std::wstrings" ) {
            first = std::wstring( L"hello world" );
            second = std::wstring( L"ciao mondo" );
            REQUIRE( first.bstrVal != second.bstrVal );
        }

        REQUIRE( first != second );
    }
}
