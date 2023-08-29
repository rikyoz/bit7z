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

auto ConvertStringToBSTR( const std::string& str ) -> BSTR {
    const int wide_length = ::MultiByteToWideChar( CP_ACP, 0 /* no flags */,
                                                   str.data(), static_cast<int>(str.length()),
                                                   nullptr, 0 );

    BSTR wide_string = ::SysAllocStringLen( nullptr, wide_length );
    ::MultiByteToWideChar( CP_ACP, 0 /* no flags */,
                           str.data(), static_cast<int>(str.length()),
                           wide_string, wide_length );
    return wide_string;
}

#endif

using namespace std;
using namespace bit7z;

constexpr auto test_wide_string = L"abcdefghijklmnopqrstuvwxyz0123456789";
constexpr auto test_tstring = BIT7Z_STRING( "abcdefghijklmnopqrstuvwxyz0123456789" );
constexpr auto test_native_string = BIT7Z_NATIVE_STRING( "abcdefghijklmnopqrstuvwxyz0123456789" );

constexpr auto test_input_encoding = L"\u30a6\u30a9\u30eb\u30b0\u30e9\u30a4\u30e2\u30f3"; // ウォルグライモン
constexpr auto test_output_encoding = BIT7Z_STRING( "\u30a6\u30a9\u30eb\u30b0\u30e9\u30a4\u30e2\u30f3" ); // ウォルグライモン

void check_variant_type( const BitPropVariant& prop_variant, BitPropVariantType type ) {
    REQUIRE( prop_variant.type() == type );
    REQUIRE( prop_variant.isEmpty() == ( type == BitPropVariantType::Empty ) );
    REQUIRE( prop_variant.isBool() == ( type == BitPropVariantType::Bool ) );
    REQUIRE( prop_variant.isString() == ( type == BitPropVariantType::String ) );
    REQUIRE( prop_variant.isUInt8() == ( type == BitPropVariantType::UInt8 ||
                                         type == BitPropVariantType::UInt16 ||
                                         type == BitPropVariantType::UInt32 ||
                                         type == BitPropVariantType::UInt64 ) );
    REQUIRE( prop_variant.isUInt16() == ( type == BitPropVariantType::UInt16 ||
                                          type == BitPropVariantType::UInt32 ||
                                          type == BitPropVariantType::UInt64 ) );
    REQUIRE( prop_variant.isUInt32() == ( type == BitPropVariantType::UInt32 ||
                                          type == BitPropVariantType::UInt64 ) );
    REQUIRE( prop_variant.isUInt64() == ( type == BitPropVariantType::UInt64 ) );
    REQUIRE( prop_variant.isInt8() == ( type == BitPropVariantType::Int8 ||
                                        type == BitPropVariantType::Int16 ||
                                        type == BitPropVariantType::Int32 ||
                                        type == BitPropVariantType::Int64 ) );
    REQUIRE( prop_variant.isInt16() == ( type == BitPropVariantType::Int16 ||
                                         type == BitPropVariantType::Int32 ||
                                         type == BitPropVariantType::Int64 ) );
    REQUIRE( prop_variant.isInt32() == ( type == BitPropVariantType::Int32 ||
                                         type == BitPropVariantType::Int64 ) );
    REQUIRE( prop_variant.isInt64() == ( type == BitPropVariantType::Int64 ) );
    REQUIRE( prop_variant.isFileTime() == ( type == BitPropVariantType::FileTime ) );

    if ( type != BitPropVariantType::Bool ) {
        REQUIRE_THROWS( prop_variant.getBool() );
    }
    if ( type != BitPropVariantType::String ) {
        REQUIRE_THROWS( prop_variant.getString() );
        REQUIRE_THROWS( prop_variant.getNativeString() );
    }

    if ( type != BitPropVariantType::UInt64 ) {
        REQUIRE_THROWS( prop_variant.getUInt64() );
        if ( type != BitPropVariantType::UInt32 ) {
            REQUIRE_THROWS( prop_variant.getUInt32() );
            if ( type != BitPropVariantType::UInt16 ) {
                REQUIRE_THROWS( prop_variant.getUInt16() );
                if ( type != BitPropVariantType::UInt8 ) {
                    REQUIRE_THROWS( prop_variant.getUInt8() );
                }
            }
        }
    }

    if ( type != BitPropVariantType::Int64 ) {
        REQUIRE_THROWS( prop_variant.getInt64() );
        if ( type != BitPropVariantType::Int32 ) {
            REQUIRE_THROWS( prop_variant.getInt32() );
            if ( type != BitPropVariantType::Int16 ) {
                REQUIRE_THROWS( prop_variant.getInt16() );
                if ( type != BitPropVariantType::Int8 ) {
                    REQUIRE_THROWS( prop_variant.getInt8() );
                }
            }
        }
    }

    if ( type != BitPropVariantType::FileTime ) {
        REQUIRE_THROWS( prop_variant.getFileTime() );
    }
}

TEST_CASE( "BitPropVariant: Empty variant", "[BitPropVariant][empty]" ) {
    BitPropVariant prop_variant;
    REQUIRE( prop_variant.vt == VT_EMPTY );
    REQUIRE( prop_variant.toString().empty() );

    check_variant_type( prop_variant, BitPropVariantType::Empty );
    REQUIRE_NOTHROW( prop_variant.clear() );
    REQUIRE( prop_variant.isEmpty() ); // still empty after clear
}

TEST_CASE( "BitPropVariant: Boolean variant", "[BitPropVariant][boolean]" ) {
    BitPropVariant prop_variant;

    SECTION( "Setting to true" ) {
        SECTION( "Using the constructor" ) {
            prop_variant = BitPropVariant( true );
        }

        SECTION( "Using manual assignment" ) {
            prop_variant.vt = VT_BOOL;
            prop_variant.boolVal = VARIANT_TRUE;
        }

        SECTION( "Using assignment" ) {
            prop_variant = true;
        }

        SECTION( "Using double assignment" ) {
            prop_variant = false;
            REQUIRE( !prop_variant.getBool() );
            prop_variant = true;
        }

        SECTION( "Using double assignment (different type)" ) {
            prop_variant = BIT7Z_STRING( "hello world!" );
            (void)prop_variant;
            prop_variant = true;
        }

        REQUIRE( prop_variant.getBool() );
        REQUIRE( prop_variant.toString() == BIT7Z_STRING( "true" ) );
    }

    SECTION( "Setting to false" ) {
        SECTION( "Using the constructor" ) {
            prop_variant = BitPropVariant( false );
        }

        SECTION( "Using manual assignment" ) {
            prop_variant.vt = VT_BOOL;
            prop_variant.boolVal = VARIANT_FALSE;
        }

        SECTION( "Using assignment" ) {
            prop_variant = false;
        }

        SECTION( "Using double assignment" ) {
            prop_variant = true;
            REQUIRE( prop_variant.getBool() );
            prop_variant = false;
        }

        SECTION( "Using double assignment (different type)" ) {
            prop_variant = BIT7Z_STRING( "hello world!" );
            (void)prop_variant;
            prop_variant = false;
        }

        REQUIRE( !prop_variant.getBool() );
        REQUIRE( prop_variant.toString() == BIT7Z_STRING( "false" ) );
    }

    check_variant_type( prop_variant, BitPropVariantType::Bool );
    REQUIRE_NOTHROW( prop_variant.clear() );
    REQUIRE( prop_variant.isEmpty() );
}

TEST_CASE( "BitPropVariant: String variant", "[BitPropVariant][string]" ) {
    BitPropVariant prop_variant;

    SECTION( "Initializing with an empty string" ) {
        SECTION( "Using the constructor (with empty wstring)" ) {
            prop_variant = BitPropVariant( wstring() );
        }

        SECTION( "Using the constructor (with empty C wide string)" ) {
            prop_variant = BitPropVariant( L"" );
        }

        SECTION( "Using the constructor (with nullptr C wide string)" ) {
            prop_variant = BitPropVariant( nullptr );
        }

        SECTION( "Manually setting to nullptr" ) {
            prop_variant.vt = VT_BSTR;
            prop_variant.bstrVal = nullptr; //semantically equivalent to empty string
        }

        SECTION( "Assigning an empty wstring" ) {
            prop_variant = wstring();
        }

        SECTION( "Assigning an empty C wide string" ) {
            prop_variant = L"";
        }

        SECTION( "Assigning a nullptr C wide string" ) { //equivalent to empty bstr
            prop_variant = nullptr; //nullptr const wchar_t* assignment
        }

        SECTION( "Double assignment" ) {
            prop_variant = L"lorem ipsum";
            REQUIRE( !prop_variant.getString().empty() );
            prop_variant = L"";
        }

        REQUIRE( prop_variant.getString().empty() );
        REQUIRE( prop_variant.getNativeString().empty() );
        REQUIRE( prop_variant.toString().empty() );
    }

    SECTION( "Initializing with a non-empty string" ) {
        SECTION( "Using the constructor (with a wstring)" ) {
            prop_variant = BitPropVariant( std::wstring( test_wide_string ) );
        }

        SECTION( "Using the constructor (with a C string)" ) {
            prop_variant = BitPropVariant( test_wide_string );
        }

#ifdef _WIN32
        SECTION( "Manually setting it" ) {
            prop_variant.vt = VT_BSTR;
#ifdef BIT7Z_USE_NATIVE_STRING
            // Note: flawfinder complains about using wcslen on a possibly non-null terminating string,
            // but test_tstring is guaranteed to be a null-terminated string!
            const auto test_tstring_size = static_cast< UINT >( wcslen( test_tstring ) ); // flawfinder: ignore
            prop_variant.bstrVal = SysAllocStringLen( test_tstring, test_tstring_size );
#else
            prop_variant.bstrVal = ConvertStringToBSTR( test_tstring );
#endif
        }
#endif

        SECTION( "Assigning a wstring" ) {
            prop_variant = std::wstring( test_wide_string );
        }

        SECTION( "Assigning a C string" ) {
            prop_variant = test_wide_string;
        }

        SECTION( "Double assignment" ) {
            prop_variant = L"lorem ipsum";
            REQUIRE( prop_variant.getString() == BIT7Z_STRING( "lorem ipsum" ) );
            prop_variant = test_wide_string;
        }

        REQUIRE( prop_variant.bstrVal != nullptr );
        REQUIRE( prop_variant.getString() == test_tstring );
        REQUIRE( prop_variant.getNativeString() == test_native_string );
        REQUIRE( prop_variant.toString() == test_tstring );
    }

#ifndef BIT7Z_TESTS_LEGACY_ENCODING
    SECTION( "Initializing with non-ASCII encoded string" ) {
        SECTION( "String literal constructor" ) {
            prop_variant = BitPropVariant{ test_input_encoding };
        }

        SECTION( "Wide string constructor" ) {
            prop_variant = BitPropVariant{ std::wstring( test_input_encoding ) };
        }

        auto encoded_tstring = prop_variant.getString();
        REQUIRE( encoded_tstring == test_output_encoding );
    }
#endif

    check_variant_type( prop_variant, BitPropVariantType::String );
    REQUIRE_NOTHROW( prop_variant.clear() );
    REQUIRE( prop_variant.isEmpty() );
    REQUIRE( prop_variant.bstrVal == nullptr );
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
    BitPropVariant prop_variant;

    TestType value = GENERATE( 0, 42, std::numeric_limits< TestType >::max() );

    SECTION( "Using the unsigned integer constructor" ) {
        prop_variant = BitPropVariant( value );
    }

    SECTION( "Manually setting the unsigned integer value" ) {
        manually_set_variant( prop_variant, value );
    }

    SECTION( "Assigning the unsigned integer value" ) {
        prop_variant = value;
    }

    SECTION( "Double assignment" ) {
        prop_variant = static_cast< uint64_t >( 84ull ); // NOLINT(*-magic-numbers)
        REQUIRE( prop_variant.getUInt64() == 84ull );
        prop_variant = value;
    }

    REQUIRE( get_value< TestType >( prop_variant ) == value );
    REQUIRE( prop_variant.toString() == uint_to_tstring( value ) );

    REQUIRE( prop_variant.type() == variant_type< TestType >() );
    REQUIRE( !prop_variant.isEmpty() );
    REQUIRE( !prop_variant.isBool() );
    REQUIRE( !prop_variant.isString() );

    // The variant is an UInt8 only if the TestType size is 8 bits.
    REQUIRE( prop_variant.isUInt8() == ( sizeof( TestType ) == 1 ) );

    // The variant is an UInt16 only if the TestType size is at most 16 bits.
    REQUIRE( prop_variant.isUInt16() == ( sizeof( TestType ) <= 2 ) );

    // The variant is an UInt32 only if the TestType size is at most 32 bits.
    REQUIRE( prop_variant.isUInt32() == ( sizeof( TestType ) <= 4 ) ); //-V112

    // All unsigned types can be contained in a UInt64.
    REQUIRE( prop_variant.isUInt64() );
    REQUIRE( !prop_variant.isInt8() );
    REQUIRE( !prop_variant.isInt16() );
    REQUIRE( !prop_variant.isInt32() );
    REQUIRE( !prop_variant.isInt64() );
    REQUIRE( !prop_variant.isFileTime() );
    REQUIRE_THROWS( prop_variant.getString() );
    REQUIRE_THROWS( prop_variant.getNativeString() );
    REQUIRE_THROWS( prop_variant.getBool() );
    REQUIRE_THROWS( prop_variant.getInt8() );
    REQUIRE_THROWS( prop_variant.getInt16() );
    REQUIRE_THROWS( prop_variant.getInt32() );
    REQUIRE_THROWS( prop_variant.getInt64() );
    REQUIRE_THROWS( prop_variant.getFileTime() );
    if ( sizeof( TestType ) == 1 ) {
        REQUIRE( prop_variant.getUInt8() == prop_variant.getUInt16() );
        REQUIRE( prop_variant.getUInt8() == prop_variant.getUInt32() );
        REQUIRE( prop_variant.getUInt8() == prop_variant.getUInt64() );
    } else {
        REQUIRE_THROWS( prop_variant.getUInt8() );
    }
    if ( sizeof( TestType ) <= 2 ) {
        REQUIRE( prop_variant.getUInt16() == prop_variant.getUInt32() );
        REQUIRE( prop_variant.getUInt16() == prop_variant.getUInt64() );
    } else {
        REQUIRE_THROWS( prop_variant.getUInt16() );
    }
    if ( sizeof( TestType ) <= 4 ) { //-V112
        REQUIRE( prop_variant.getUInt32() == prop_variant.getUInt64() );
    } else {
        REQUIRE_THROWS( prop_variant.getUInt32() );
    }
    REQUIRE_NOTHROW( prop_variant.clear() );
    REQUIRE( prop_variant.isEmpty() );
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
    BitPropVariant prop_variant;

    TestType value = GENERATE( std::numeric_limits< TestType >::min(),
                               -42,
                               0,
                               42,
                               std::numeric_limits< TestType >::max() );

    SECTION( "Using the signed integer constructor" ) {
        prop_variant = BitPropVariant( value );
    }

    SECTION( "Manually setting the signed integer value" ) {
        manually_set_variant( prop_variant, value );
    }

    SECTION( "Assigning the signed integer value" ) {
        prop_variant = value;
    }

    SECTION( "Double assignment" ) {
        prop_variant = static_cast< uint64_t >( 84ull ); // NOLINT(*-magic-numbers)
        REQUIRE( prop_variant.getUInt64() == 84ull );
        prop_variant = value;
    }

    REQUIRE( get_value< TestType >( prop_variant ) == value );
    REQUIRE( prop_variant.toString() == int_to_tstring( value ) );

    REQUIRE( prop_variant.type() == variant_type< TestType >() );
    REQUIRE( !prop_variant.isEmpty() );
    REQUIRE( !prop_variant.isBool() );
    REQUIRE( !prop_variant.isString() );

    // The variant is an Int8 only if the TestType size is 8 bits.
    REQUIRE( prop_variant.isInt8() == ( sizeof( TestType ) == 1 ) );

    // The variant is an Int16 only if the TestType size is at most 16 bits.
    REQUIRE( prop_variant.isInt16() == ( sizeof( TestType ) <= 2 ) );

    // The variant is an Int32 only if the TestType size is at most 32 bits.
    REQUIRE( prop_variant.isInt32() == ( sizeof( TestType ) <= 4 ) ); //-V112

    // All signed types can be contained in an Int64.
    REQUIRE( prop_variant.isInt64() );

    REQUIRE( !prop_variant.isUInt8() );
    REQUIRE( !prop_variant.isUInt16() );
    REQUIRE( !prop_variant.isUInt32() );
    REQUIRE( !prop_variant.isUInt64() );
    REQUIRE( !prop_variant.isFileTime() );
    REQUIRE_THROWS( prop_variant.getString() );
    REQUIRE_THROWS( prop_variant.getNativeString() );
    REQUIRE_THROWS( prop_variant.getBool() );
    REQUIRE_THROWS( prop_variant.getUInt8() );
    REQUIRE_THROWS( prop_variant.getUInt16() );
    REQUIRE_THROWS( prop_variant.getUInt32() );
    REQUIRE_THROWS( prop_variant.getUInt64() );
    REQUIRE_THROWS( prop_variant.getFileTime() );
    if ( sizeof( TestType ) == 1 ) {
        REQUIRE( prop_variant.getInt8() == prop_variant.getInt16() );
        REQUIRE( prop_variant.getInt8() == prop_variant.getInt32() );
        REQUIRE( prop_variant.getInt8() == prop_variant.getInt64() );
    } else {
        REQUIRE_THROWS( prop_variant.getInt8() );
    }
    if ( sizeof( TestType ) <= 2 ) {
        REQUIRE( prop_variant.getInt16() == prop_variant.getInt32() );
        REQUIRE( prop_variant.getInt16() == prop_variant.getInt64() );
    } else {
        REQUIRE_THROWS( prop_variant.getInt16() );
    }
    if ( sizeof( TestType ) <= 4 ) { //-V112
        REQUIRE( prop_variant.getInt32() == prop_variant.getInt64() );
    } else {
        REQUIRE_THROWS( prop_variant.getInt32() );
    }
    REQUIRE_NOTHROW( prop_variant.clear() );
    REQUIRE( prop_variant.isEmpty() );
}

TEST_CASE( "BitPropVariant: FILETIME variant", "[BitPropVariant][FILETIME]" ) {
    //NOLINTNEXTLINE(*-magic-numbers)
    FILETIME value{ 3017121792, 30269298 }; // 21 December 2012, 12:00
    BitPropVariant prop_variant;

    SECTION( "Using the constructor" ) {
        prop_variant = BitPropVariant( value );
    }

    SECTION( "Manually setting" ) {
        prop_variant.vt = VT_FILETIME;
        prop_variant.filetime = value;
    }

    SECTION( "Assignment" ) {
        prop_variant = value;
    }

    SECTION( "Double assignment" ) {
        prop_variant = static_cast< uint64_t >( 84ull ); // NOLINT(*-magic-numbers)
        REQUIRE( prop_variant.getUInt64() == 84ull );
        prop_variant = value;
    }

    auto result = prop_variant.getFileTime();
    REQUIRE( std::memcmp( &result, &value, sizeof( FILETIME ) ) == 0 );

    check_variant_type( prop_variant, BitPropVariantType::FileTime );
    REQUIRE_NOTHROW( prop_variant.clear() );
    REQUIRE( prop_variant.isEmpty() );
}

TEST_CASE( "BitPropVariant: Copying string variants", "[BitPropVariant][copy]" ) {
    const BitPropVariant prop_variant{ std::wstring( test_wide_string ) };

    SECTION( "Copy constructor" ) {
        const BitPropVariant copy_var( prop_variant ); //copy constructor
        REQUIRE( !copy_var.isEmpty() );
        REQUIRE( copy_var.vt == prop_variant.vt );
        REQUIRE( copy_var.type() == prop_variant.type() );
        REQUIRE( copy_var.bstrVal != prop_variant.bstrVal );
        REQUIRE( wcscmp( copy_var.bstrVal, prop_variant.bstrVal ) == 0 );
        REQUIRE( copy_var.getString() == prop_variant.getString() );
        REQUIRE( copy_var.getNativeString() == prop_variant.getNativeString() );
    }

    SECTION( "Copy assignment" ) {
        const BitPropVariant copy_var = prop_variant;
        REQUIRE( !copy_var.isEmpty() );
        REQUIRE( copy_var.vt == prop_variant.vt );
        REQUIRE( copy_var.type() == prop_variant.type() );
        REQUIRE( copy_var.bstrVal != prop_variant.bstrVal );
        REQUIRE( wcscmp( copy_var.bstrVal, prop_variant.bstrVal ) == 0 );
        REQUIRE( copy_var.getString() == prop_variant.getString() );
        REQUIRE( copy_var.getNativeString() == prop_variant.getNativeString() );
    }
}

TEST_CASE( "BitPropVariant: Moving string variants", "[BitPropVariant][copy]" ) {
    BitPropVariant prop_variant{ std::wstring( test_wide_string ) };

    SECTION( "Move constructor" ) {
        // The move may invalidate propvariant.bstrVal (make it nullptr), so we copy the pointer
        // and check the moved variant uses the same pointer!
        BSTR test_bstrVal = prop_variant.bstrVal;
        const BitPropVariant move_var( std::move( prop_variant ) );
        REQUIRE( !move_var.isEmpty() );
        REQUIRE( move_var.vt == VT_BSTR );
        REQUIRE( move_var.bstrVal ==
                 test_bstrVal ); //move_var should point to the same BSTR object of the original prop_variant!
        REQUIRE( move_var.getString() == test_tstring );
        REQUIRE( move_var.getNativeString() == test_native_string );
    }

    SECTION( "Move assignment" ) {
        // The move may invalidate propvariant.bstrVal (make it nullptr), so we copy the pointer
        // and check if the moved variant uses the same pointer!
        BSTR test_bstrVal = prop_variant.bstrVal; // NOLINT(bugprone-use-after-move)
        const BitPropVariant move_var = std::move( prop_variant ); // cppcheck-suppress accessMoved
        REQUIRE( !move_var.isEmpty() );
        REQUIRE( move_var.vt == VT_BSTR );
        REQUIRE( move_var.bstrVal ==
                 test_bstrVal ); //move_var should point to the same BSTR object of the original prop_variant!
        REQUIRE( move_var.getString() == test_tstring );
        REQUIRE( move_var.getNativeString() == test_native_string );
    }
}

TEST_CASE( "BitPropVariant: Equality operator", "[bitpropvariant][equality]" ) {
    BitPropVariant a;
    BitPropVariant b;
    REQUIRE( a == b );

    SECTION( "Comparing equal variants" ) {
        SECTION( "Same signed integers" ) {
            a = 42; // NOLINT(*-magic-numbers)
            b = 42; // NOLINT(*-magic-numbers)
        }

        SECTION( "Same unsigned integers" ) {
            a = 42u; // NOLINT(*-magic-numbers)
            b = 42u; // NOLINT(*-magic-numbers)
        }

        SECTION( "Same booleans (true)" ) {
            a = true;
            b = true;
        }

        SECTION( "Same booleans (true)" ) {
            a = false;
            b = false;
        }

        SECTION( "Same C strings" ) {
            a = L"hello world";
            b = L"hello world";
            REQUIRE( a.bstrVal != b.bstrVal );
        }

        SECTION( "Same std::wstrings" ) {
            a = std::wstring( test_wide_string );
            b = std::wstring( test_wide_string );
            REQUIRE( a.bstrVal != b.bstrVal );
        }

        REQUIRE( a == b );
    }

    SECTION( "Comparing different variants" ) {
        SECTION( "Different signed integers" ) {
            a = 42; // NOLINT(*-magic-numbers)
            b = 84; // NOLINT(*-magic-numbers)
        }

        SECTION( "Different unsigned integers" ) {
            a = 42u; // NOLINT(*-magic-numbers)
            b = 84u; // NOLINT(*-magic-numbers)
        }

        SECTION( "Integers with different signedness" ) {
            a = 42; // NOLINT(*-magic-numbers)
            b = 42u; // NOLINT(*-magic-numbers)
            //unsigned 42 (b) is different from a signed 42 (a)!
        }

        SECTION( "Different booleans (1)" ) {
            a = true;
            b = false;
        }

        SECTION( "Different booleans (2)" ) {
            a = false;
            b = true;
        }

        SECTION( "Integer and a string" ) {
            a = 42; // NOLINT(*-magic-numbers)
            b = L"ciao mondo";
        }

        SECTION( "Different C strings" ) {
            a = L"hello world";
            b = L"ciao mondo";
            REQUIRE( a.bstrVal != b.bstrVal );
        }

        SECTION( "Different std::wstrings" ) {
            a = std::wstring( L"hello world" );
            b = std::wstring( L"ciao mondo" );
            REQUIRE( a.bstrVal != b.bstrVal );
        }

        REQUIRE( a != b );
    }
}
