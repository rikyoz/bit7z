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

#include <internal/cbufferinstream.hpp>

#include <limits>

using bit7z::byte_t;
using bit7z::buffer_t;
using bit7z::CBufferInStream;

//-V::2008 (Suppressing warnings for cyclomatic complexity in PVS-Studio)

TEST_CASE( "CBufferInStream: Seeking a buffer stream with no content", "[cbufferinstream][seeking]" ) {
    const size_t buffer_size = GENERATE( 0, 1, 5, 42, 1024 * 1024 );

    DYNAMIC_SECTION( "Streaming a buffer of size " << buffer_size ) { //-V128
        const buffer_t buffer( buffer_size );
        CBufferInStream in_stream{ buffer };
        UInt64 new_position{ 0 };

        SECTION( "Invalid seek origin" ) {
            REQUIRE( in_stream.Seek( 0, 3, &new_position ) == STG_E_INVALIDFUNCTION );
            REQUIRE( new_position == 0 );
        }

        SECTION( "Seeking without reading the new position" ) {
            REQUIRE( in_stream.Seek( 0, STREAM_SEEK_SET, nullptr ) == S_OK );
            REQUIRE( in_stream.Seek( 0, STREAM_SEEK_CUR, nullptr ) == S_OK );
            REQUIRE( in_stream.Seek( 0, STREAM_SEEK_END, nullptr ) == S_OK );
        }

        SECTION( "Seeking from the beginning of the stream (STREAM_SEEK_SET)" ) {
            SECTION( "To the beginning of the stream (Boundary Value Analysis)" ) {
                REQUIRE( in_stream.Seek( -1, STREAM_SEEK_SET, &new_position ) == HRESULT_WIN32_ERROR_NEGATIVE_SEEK );
                REQUIRE( new_position == 0 );

                REQUIRE( in_stream.Seek( 0, STREAM_SEEK_SET, &new_position ) == S_OK );
                REQUIRE( new_position == 0 );

                if ( !buffer.empty() ) {
                    REQUIRE( in_stream.Seek( 1, STREAM_SEEK_SET, &new_position ) == S_OK );
                    REQUIRE( new_position == 1 );
                }
            }

            SECTION( "To the end of the stream (Boundary Value Analysis)" ) {
                if ( !buffer.empty() ) {
                    REQUIRE( in_stream.Seek( buffer_size - 1, STREAM_SEEK_SET, &new_position ) == S_OK );
                    REQUIRE( new_position == buffer_size - 1 );
                }

                // Note: We can seek one element past the last value in the buffer;
                //       this is equivalent to having reached the buffer's end().
                REQUIRE( in_stream.Seek( buffer_size, STREAM_SEEK_SET, &new_position ) == S_OK );
                REQUIRE( new_position == buffer_size );

                REQUIRE( in_stream.Seek( buffer_size + 1, STREAM_SEEK_SET, &new_position ) == E_INVALIDARG );
                REQUIRE( new_position == buffer_size ); //old value, not changed
            }

            SECTION( "To an offset outside the valid range (Boundary Value Analysis)" ) {
                REQUIRE( in_stream.Seek( std::numeric_limits< Int64 >::max(), STREAM_SEEK_SET, &new_position ) ==
                         E_INVALIDARG );
                REQUIRE( new_position == 0 );

                REQUIRE( in_stream.Seek( std::numeric_limits< Int64 >::min(), STREAM_SEEK_SET, &new_position ) ==
                         HRESULT_WIN32_ERROR_NEGATIVE_SEEK );
                REQUIRE( new_position == 0 );
            }

            if ( buffer.size() > 1 ) {
                SECTION( "To the middle of the stream" ) {
                    const Int64 mid_offset = static_cast< Int64 >( buffer_size ) / 2;
                    REQUIRE( in_stream.Seek( mid_offset, STREAM_SEEK_SET, &new_position ) == S_OK );
                    REQUIRE( new_position == mid_offset );
                }
            }
        }

        SECTION( "Seeking from the end of the stream (STREAM_SEEK_END)" ) {
            SECTION( "To the beginning of the stream (Boundary Value Analysis)" ) {
                const Int64 offset = -static_cast< Int64 >( buffer_size );
                REQUIRE( in_stream.Seek( offset - 1, STREAM_SEEK_END, &new_position ) ==
                         HRESULT_WIN32_ERROR_NEGATIVE_SEEK );
                REQUIRE( new_position == 0 );

                // Note: The end of the stream is one element past the last one in the buffer,
                //       so if we seek to an offset equal to the size of the buffer,
                //       we reached the first element in the buffer!
                REQUIRE( in_stream.Seek( offset, STREAM_SEEK_END, &new_position ) == S_OK );
                REQUIRE( new_position == 0 );

                if ( !buffer.empty() ) {
                    REQUIRE( in_stream.Seek( offset + 1, STREAM_SEEK_END, &new_position ) == S_OK );
                    REQUIRE( new_position == 1 );
                }
            }

            SECTION( "To the end of the stream (Boundary Value Analysis)" ) {
                if ( !buffer.empty() ) {
                    REQUIRE( in_stream.Seek( -1, STREAM_SEEK_END, &new_position ) == S_OK );
                    REQUIRE( new_position == buffer_size - 1 );
                }

                REQUIRE( in_stream.Seek( 0, STREAM_SEEK_END, &new_position ) == S_OK );
                REQUIRE( new_position == buffer_size );

                new_position = 0; // Resetting the position value
                REQUIRE( in_stream.Seek( 1, STREAM_SEEK_END, &new_position ) == E_INVALIDARG );
                REQUIRE( new_position == 0 );
            }

            SECTION( "To an offset outside the valid range (Boundary Value Analysis)" ) {
                REQUIRE( in_stream.Seek( std::numeric_limits< Int64 >::max(), STREAM_SEEK_END, &new_position ) ==
                         E_INVALIDARG );
                REQUIRE( new_position == 0 );

                REQUIRE( in_stream.Seek( std::numeric_limits< Int64 >::min(), STREAM_SEEK_END, &new_position ) ==
                         HRESULT_WIN32_ERROR_NEGATIVE_SEEK );
                REQUIRE( new_position == 0 );
            }

            if ( buffer.size() > 1 ) {
                SECTION( "To the middle of the stream" ) {
                    // Note: Calculating the offset of the midpoint item of the buffer from the end of the buffer stream
                    //       (which is one element past the last item of the buffer).
                    const Int64 mid_offset = -( ( static_cast< Int64 >( buffer_size ) / 2 ) + 1 );
                    REQUIRE( in_stream.Seek( mid_offset, STREAM_SEEK_END, &new_position ) == S_OK );
                    REQUIRE( new_position == static_cast< Int64 >( buffer_size ) + mid_offset );
                }
            }
        }

        SECTION( "Seeking from the current position in the stream (STREAM_SEEK_CUR)" ) {
            SECTION( "The default current position in the stream is at the start" ) {
                REQUIRE( in_stream.Seek( 0, STREAM_SEEK_CUR, &new_position ) == S_OK );
                REQUIRE( new_position == 0 );
            }

            SECTION( "After seeking to the end of the stream, the current seek position is correct" ) {
                REQUIRE( in_stream.Seek( 0, STREAM_SEEK_END, nullptr ) == S_OK );
                REQUIRE( in_stream.Seek( 0, STREAM_SEEK_CUR, &new_position ) == S_OK );
                REQUIRE( new_position == buffer_size );
            }

            if ( buffer.size() > 1 ) {
                SECTION( "Seeking from the midpoint of the stream" ) {
                    // First, we seek to the midpoint of the stream
                    const Int64 mid_offset = static_cast< Int64 >( buffer_size ) / 2;
                    INFO( "Middle offset value: " << mid_offset )
                    REQUIRE( in_stream.Seek( mid_offset, STREAM_SEEK_SET, nullptr ) == S_OK );
                    REQUIRE( in_stream.Seek( 0, STREAM_SEEK_CUR, &new_position ) == S_OK );
                    REQUIRE( new_position == mid_offset );

                    SECTION( "To the beginning of the stream (Boundary Value Analysis)" ) {
                        REQUIRE( in_stream.Seek( -mid_offset, STREAM_SEEK_CUR, &new_position ) == S_OK );
                        REQUIRE( new_position == 0 );
                    }

                    SECTION( "To a position before the beginning of the stream (Boundary Value Analysis)" ) {
                        REQUIRE( in_stream.Seek( -mid_offset - 1, STREAM_SEEK_CUR, &new_position ) ==
                                 HRESULT_WIN32_ERROR_NEGATIVE_SEEK );
                        REQUIRE( new_position == mid_offset ); // The position within the stream didn't change!
                    }

                    SECTION( "To the end of the stream (Boundary Value Analysis)" ) {
                        const auto end_offset = ( buffer_size % 2 == 0 ? mid_offset : mid_offset + 1 );
                        REQUIRE( in_stream.Seek( end_offset, STREAM_SEEK_CUR, &new_position ) == S_OK );
                        REQUIRE( new_position == buffer_size );
                    }

                    SECTION( "To a position after the end of the stream (Boundary Value Analysis)" ) {
                        const auto end_offset = ( buffer_size % 2 == 0 ? mid_offset : mid_offset + 1 );
                        REQUIRE( in_stream.Seek( end_offset + 1, STREAM_SEEK_CUR, &new_position ) == E_INVALIDARG );
                        REQUIRE( new_position == mid_offset );
                    }
                }
            }
        }

        //Reset position
        REQUIRE( in_stream.Seek( 0, STREAM_SEEK_SET, &new_position ) == S_OK );
        REQUIRE( new_position == 0 );
    }
}

TEST_CASE( "CBufferInStream: Reading an empty buffer stream", "[cbufferinstream][reading]" ) {
    const buffer_t buffer{};
    CBufferInStream in_stream{ buffer };
    UInt32 processed_size{ 0 };

    auto data = static_cast< byte_t >( 'A' );

    SECTION( "Reading only one character" ) {
        REQUIRE( in_stream.Read( &data, 1, &processed_size ) == S_OK );
        REQUIRE( processed_size == 0 );
        REQUIRE( data == 'A' );
    }

    SECTION( "Trying to read more characters than the ones in the buffer" ) {
        REQUIRE( in_stream.Read( &data, 42, &processed_size ) == S_OK );
        REQUIRE( processed_size == 0 );
        REQUIRE( data == 'A' ); // data was not changed!
    }
}

TEST_CASE( "CBufferInStream: Reading a single-value buffer stream", "[cbufferinstream][reading]" ) {
    const buffer_t buffer{ 'R' };
    CBufferInStream in_stream{ buffer };
    UInt32 processed_size{ 0 };

    auto data = static_cast< byte_t >( 'A' );

    SECTION( "Reading only one character" ) {
        REQUIRE( in_stream.Read( &data, 1, &processed_size ) == S_OK );
        REQUIRE( processed_size == 1 );
        REQUIRE( data == 'R' );
    }

    SECTION( "Trying to read more characters than the ones in the buffer" ) {
        REQUIRE( in_stream.Read( &data, 42, &processed_size ) == S_OK );
        REQUIRE( processed_size == 1 );
        REQUIRE( data == 'R' );
    }
}

TEST_CASE( "CBufferInStream: Reading a buffer stream", "[cbufferinstream][reading]" ) {
    const buffer_t buffer { 'H', 'e', 'l', 'l', 'o', ' ', 'W', 'o', 'r', 'l', 'd', '!' }; // Hello World!
    CBufferInStream in_stream{ buffer };
    UInt32 processed_size{ 0 };

    SECTION( "Reading chunks of bytes" ) {
        buffer_t result;

        SECTION( "Reading the whole buffer stream" ) {
            const size_t read_size = buffer.size();
            result.resize( read_size, 0 );
            REQUIRE( in_stream.Read( &result[ 0 ], read_size, &processed_size ) == S_OK );
            REQUIRE( processed_size == read_size );
            REQUIRE( std::memcmp( result.data(), "Hello World!", read_size ) == 0 );
        }

        SECTION( "Reading first half of the buffer stream" ) {
            const size_t read_size = buffer.size() / 2;
            result.resize( read_size, 0 );
            REQUIRE( in_stream.Read( &result[ 0 ], read_size, &processed_size ) == S_OK );
            REQUIRE( processed_size == read_size );
            REQUIRE( std::memcmp( result.data(), "Hello ", read_size ) == 0 );
        }

        SECTION( "Reading last half of the buffer stream" ) {
            const size_t read_size = buffer.size() / 2;

            UInt64 new_position{ 0 };
            REQUIRE( in_stream.Seek( read_size, STREAM_SEEK_SET, &new_position ) == S_OK );
            REQUIRE( new_position == read_size );

            result.resize( read_size, 0 );
            REQUIRE( in_stream.Read( &result[ 0 ], read_size, &processed_size ) == S_OK );
            REQUIRE( processed_size == read_size );
            REQUIRE( std::memcmp( result.data(), "World!", read_size ) == 0 );
        }

        SECTION( "Reading the middle part of the buffer stream" ) {
            const Int64 read_offset = static_cast< Int64 >( buffer.size() ) / 4; //-V112
            UInt64 new_position{ 0 };
            REQUIRE( in_stream.Seek( read_offset, STREAM_SEEK_SET, &new_position ) == S_OK );
            REQUIRE( new_position == read_offset );

            const size_t read_size = buffer.size() / 2;
            result.resize( read_size, 0 );
            REQUIRE( in_stream.Read( &result[ 0 ], read_size, &processed_size ) == S_OK );
            REQUIRE( processed_size == read_size );
            REQUIRE( std::memcmp( result.data(), "lo Wor", read_size ) == 0 );
        }

        SECTION( "Trying to read more characters than the ones in the input buffer" ) {
            result.resize( buffer.size(), 0 );
            REQUIRE( in_stream.Read( &result[ 0 ], buffer.size() * 2, &processed_size ) == S_OK );
            REQUIRE( processed_size == buffer.size() );
            REQUIRE( std::memcmp( result.data(), "Hello World!", buffer.size() ) == 0 );
        }
    }

    SECTION( "Reading single characters" ) {
        auto result = static_cast< byte_t >( 'A' ); // A character not in the buffer
        UInt64 new_position{ 0 };

        REQUIRE( in_stream.Seek( buffer.size() / 2, STREAM_SEEK_SET, &new_position ) == S_OK );
        REQUIRE( new_position == ( buffer.size() / 2 ) );

        REQUIRE( in_stream.Read( &result, 1, &processed_size ) == S_OK );
        REQUIRE( processed_size == 1 );
        REQUIRE( result == 'W' );

        // Seeking back to the start of the stream
        REQUIRE( in_stream.Seek( 0, STREAM_SEEK_SET, &new_position ) == S_OK );
        REQUIRE( new_position == 0 );

        REQUIRE( in_stream.Read( &result, 1, &processed_size ) == S_OK );
        REQUIRE( processed_size == 1 );
        REQUIRE( result == 'H' );

        // Seeking to the last character of the stream
        REQUIRE( in_stream.Seek( -1, STREAM_SEEK_END, &new_position ) == S_OK );
        REQUIRE( new_position == buffer.size() - 1 );

        REQUIRE( in_stream.Read( &result, 1, &processed_size ) == S_OK );
        REQUIRE( processed_size == 1 );
        REQUIRE( result == '!' );
    }

    SECTION( "Reading from the end of the stream" ) {
        UInt64 new_position{ 0 };
        REQUIRE( in_stream.Seek( 0, STREAM_SEEK_END, &new_position ) == S_OK );
        REQUIRE( new_position == buffer.size() );

        auto result = static_cast< byte_t >( 'A' ); // A character not in the buffer
        REQUIRE( in_stream.Read( &result, 1, &processed_size ) == S_OK ); // Not an error,
        REQUIRE( processed_size == 0 ); // but we didn't read anything, as expected!
        REQUIRE( result == 'A' ); // And hence, the result value was not changed!
    }

    SECTION( "Reading nothing from the stream" ) {
        auto result  = static_cast< byte_t >( 'A' ); // A character not in the buffer
        REQUIRE( in_stream.Read( &result, 0, &processed_size ) == S_OK ); // Not an error,
        REQUIRE( processed_size == 0 ); // but we didn't read anything, as expected!
        REQUIRE( result == 'A' ); // And hence, the result value was not changed!
    }
}