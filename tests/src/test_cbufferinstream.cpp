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
    const size_t bufferSize = GENERATE( 0, 1, 5, 42, 1024 * 1024 );

    DYNAMIC_SECTION( "Streaming a buffer of size " << bufferSize ) { //-V128
        const buffer_t buffer( bufferSize );
        CBufferInStream inStream{ buffer };
        UInt64 newPosition{ 0 };

        SECTION( "Invalid seek origin" ) {
            REQUIRE( inStream.Seek( 0, 3, &newPosition ) == STG_E_INVALIDFUNCTION );
            REQUIRE( newPosition == 0 );
        }

        SECTION( "Seeking without reading the new position" ) {
            REQUIRE( inStream.Seek( 0, STREAM_SEEK_SET, nullptr ) == S_OK );
            REQUIRE( inStream.Seek( 0, STREAM_SEEK_CUR, nullptr ) == S_OK );
            REQUIRE( inStream.Seek( 0, STREAM_SEEK_END, nullptr ) == S_OK );
        }

        SECTION( "Seeking from the beginning of the stream (STREAM_SEEK_SET)" ) {
            SECTION( "To the beginning of the stream (Boundary Value Analysis)" ) {
                REQUIRE( inStream.Seek( -1, STREAM_SEEK_SET, &newPosition ) == HRESULT_WIN32_ERROR_NEGATIVE_SEEK );
                REQUIRE( newPosition == 0 );

                REQUIRE( inStream.Seek( 0, STREAM_SEEK_SET, &newPosition ) == S_OK );
                REQUIRE( newPosition == 0 );

                if ( !buffer.empty() ) {
                    REQUIRE( inStream.Seek( 1, STREAM_SEEK_SET, &newPosition ) == S_OK );
                    REQUIRE( newPosition == 1 );
                }
            }

            SECTION( "To the end of the stream (Boundary Value Analysis)" ) {
                if ( !buffer.empty() ) {
                    REQUIRE( inStream.Seek( bufferSize - 1, STREAM_SEEK_SET, &newPosition ) == S_OK );
                    REQUIRE( newPosition == bufferSize - 1 );
                }

                // Note: We can seek one element past the last value in the buffer;
                //       this is equivalent to having reached the buffer's end().
                REQUIRE( inStream.Seek( bufferSize, STREAM_SEEK_SET, &newPosition ) == S_OK );
                REQUIRE( newPosition == bufferSize );

                REQUIRE( inStream.Seek( bufferSize + 1, STREAM_SEEK_SET, &newPosition ) == E_INVALIDARG );
                REQUIRE( newPosition == bufferSize ); //old value, not changed
            }

            SECTION( "To an offset outside the valid range (Boundary Value Analysis)" ) {
                REQUIRE( inStream.Seek( std::numeric_limits< Int64 >::max(), STREAM_SEEK_SET, &newPosition ) ==
                         E_INVALIDARG );
                REQUIRE( newPosition == 0 );

                REQUIRE( inStream.Seek( std::numeric_limits< Int64 >::min(), STREAM_SEEK_SET, &newPosition ) ==
                         HRESULT_WIN32_ERROR_NEGATIVE_SEEK );
                REQUIRE( newPosition == 0 );
            }

            if ( buffer.size() > 1 ) {
                SECTION( "To the middle of the stream" ) {
                    const Int64 midOffset = static_cast< Int64 >( bufferSize ) / 2;
                    REQUIRE( inStream.Seek( midOffset, STREAM_SEEK_SET, &newPosition ) == S_OK );
                    REQUIRE( newPosition == midOffset );
                }
            }
        }

        SECTION( "Seeking from the end of the stream (STREAM_SEEK_END)" ) {
            SECTION( "To the beginning of the stream (Boundary Value Analysis)" ) {
                const Int64 offset = -static_cast< Int64 >( bufferSize );
                REQUIRE( inStream.Seek( offset - 1, STREAM_SEEK_END, &newPosition ) ==
                         HRESULT_WIN32_ERROR_NEGATIVE_SEEK );
                REQUIRE( newPosition == 0 );

                // Note: The end of the stream is one element past the last one in the buffer,
                //       so if we seek to an offset equal to the size of the buffer,
                //       we reached the first element in the buffer!
                REQUIRE( inStream.Seek( offset, STREAM_SEEK_END, &newPosition ) == S_OK );
                REQUIRE( newPosition == 0 );

                if ( !buffer.empty() ) {
                    REQUIRE( inStream.Seek( offset + 1, STREAM_SEEK_END, &newPosition ) == S_OK );
                    REQUIRE( newPosition == 1 );
                }
            }

            SECTION( "To the end of the stream (Boundary Value Analysis)" ) {
                if ( !buffer.empty() ) {
                    REQUIRE( inStream.Seek( -1, STREAM_SEEK_END, &newPosition ) == S_OK );
                    REQUIRE( newPosition == bufferSize - 1 );
                }

                REQUIRE( inStream.Seek( 0, STREAM_SEEK_END, &newPosition ) == S_OK );
                REQUIRE( newPosition == bufferSize );

                newPosition = 0; // Resetting the position value
                REQUIRE( inStream.Seek( 1, STREAM_SEEK_END, &newPosition ) == E_INVALIDARG );
                REQUIRE( newPosition == 0 );
            }

            SECTION( "To an offset outside the valid range (Boundary Value Analysis)" ) {
                REQUIRE( inStream.Seek( std::numeric_limits< Int64 >::max(), STREAM_SEEK_END, &newPosition ) ==
                         E_INVALIDARG );
                REQUIRE( newPosition == 0 );

                REQUIRE( inStream.Seek( std::numeric_limits< Int64 >::min(), STREAM_SEEK_END, &newPosition ) ==
                         HRESULT_WIN32_ERROR_NEGATIVE_SEEK );
                REQUIRE( newPosition == 0 );
            }

            if ( buffer.size() > 1 ) {
                SECTION( "To the middle of the stream" ) {
                    // Note: Calculating the offset of the midpoint item of the buffer from the end of the buffer stream
                    //       (which is one element past the last item of the buffer).
                    const Int64 midOffset = -( ( static_cast< Int64 >( bufferSize ) / 2 ) + 1 );
                    REQUIRE( inStream.Seek( midOffset, STREAM_SEEK_END, &newPosition ) == S_OK );
                    REQUIRE( newPosition == static_cast< Int64 >( bufferSize ) + midOffset );
                }
            }
        }

        SECTION( "Seeking from the current position in the stream (STREAM_SEEK_CUR)" ) {
            SECTION( "The default current position in the stream is at the start" ) {
                REQUIRE( inStream.Seek( 0, STREAM_SEEK_CUR, &newPosition ) == S_OK );
                REQUIRE( newPosition == 0 );
            }

            SECTION( "After seeking to the end of the stream, the current seek position is correct" ) {
                REQUIRE( inStream.Seek( 0, STREAM_SEEK_END, nullptr ) == S_OK );
                REQUIRE( inStream.Seek( 0, STREAM_SEEK_CUR, &newPosition ) == S_OK );
                REQUIRE( newPosition == bufferSize );
            }

            if ( buffer.size() > 1 ) {
                SECTION( "Seeking from the midpoint of the stream" ) {
                    // First, we seek to the midpoint of the stream
                    const Int64 midOffset = static_cast< Int64 >( bufferSize ) / 2;
                    INFO( "Middle offset value: " << midOffset )
                    REQUIRE( inStream.Seek( midOffset, STREAM_SEEK_SET, nullptr ) == S_OK );
                    REQUIRE( inStream.Seek( 0, STREAM_SEEK_CUR, &newPosition ) == S_OK );
                    REQUIRE( newPosition == midOffset );

                    SECTION( "To the beginning of the stream (Boundary Value Analysis)" ) {
                        REQUIRE( inStream.Seek( -midOffset, STREAM_SEEK_CUR, &newPosition ) == S_OK );
                        REQUIRE( newPosition == 0 );
                    }

                    SECTION( "To a position before the beginning of the stream (Boundary Value Analysis)" ) {
                        REQUIRE( inStream.Seek( -midOffset - 1, STREAM_SEEK_CUR, &newPosition ) ==
                                 HRESULT_WIN32_ERROR_NEGATIVE_SEEK );
                        REQUIRE( newPosition == midOffset ); // The position within the stream didn't change!
                    }

                    SECTION( "To the end of the stream (Boundary Value Analysis)" ) {
                        const auto endOffset = ( bufferSize % 2 == 0 ? midOffset : midOffset + 1 );
                        REQUIRE( inStream.Seek( endOffset, STREAM_SEEK_CUR, &newPosition ) == S_OK );
                        REQUIRE( newPosition == bufferSize );
                    }

                    SECTION( "To a position after the end of the stream (Boundary Value Analysis)" ) {
                        const auto endOffset = ( bufferSize % 2 == 0 ? midOffset : midOffset + 1 );
                        REQUIRE( inStream.Seek( endOffset + 1, STREAM_SEEK_CUR, &newPosition ) == E_INVALIDARG );
                        REQUIRE( newPosition == midOffset );
                    }
                }
            }
        }

        //Reset position
        REQUIRE( inStream.Seek( 0, STREAM_SEEK_SET, &newPosition ) == S_OK );
        REQUIRE( newPosition == 0 );
    }
}

TEST_CASE( "CBufferInStream: Reading an empty buffer stream", "[cbufferinstream][reading]" ) {
    const buffer_t buffer{};
    CBufferInStream inStream{ buffer };
    UInt32 processedSize{ 0 };

    auto data = static_cast< byte_t >( 'A' );

    SECTION( "Reading only one character" ) {
        REQUIRE( inStream.Read( &data, 1, &processedSize ) == S_OK );
        REQUIRE( processedSize == 0 );
        REQUIRE( data == static_cast< byte_t >( 'A' ) );
    }

    SECTION( "Trying to read more characters than the ones in the buffer" ) {
        REQUIRE( inStream.Read( &data, 42, &processedSize ) == S_OK );
        REQUIRE( processedSize == 0 );
        REQUIRE( data == static_cast< byte_t >( 'A' ) ); // data was not changed!
    }
}

TEST_CASE( "CBufferInStream: Reading a single-value buffer stream", "[cbufferinstream][reading]" ) {
    const buffer_t buffer{ static_cast< byte_t >( 'R' ) };
    CBufferInStream inStream{ buffer };
    UInt32 processedSize{ 0 };

    auto data = static_cast< byte_t >( 'A' );

    SECTION( "Reading only one character" ) {
        REQUIRE( inStream.Read( &data, 1, &processedSize ) == S_OK );
        REQUIRE( processedSize == 1 );
        REQUIRE( data == static_cast< byte_t >( 'R' ) );
    }

    SECTION( "Trying to read more characters than the ones in the buffer" ) {
        REQUIRE( inStream.Read( &data, 42, &processedSize ) == S_OK );
        REQUIRE( processedSize == 1 );
        REQUIRE( data == static_cast< byte_t >( 'R' ) );
    }
}

TEST_CASE( "CBufferInStream: Reading a buffer stream", "[cbufferinstream][reading]" ) {
    const buffer_t buffer{ static_cast< byte_t >( 'H' ),
                           static_cast< byte_t >( 'e' ),
                           static_cast< byte_t >( 'l' ),
                           static_cast< byte_t >( 'l' ),
                           static_cast< byte_t >( 'o' ),
                           static_cast< byte_t >( ' ' ),
                           static_cast< byte_t >( 'W' ),
                           static_cast< byte_t >( 'o' ),
                           static_cast< byte_t >( 'r' ),
                           static_cast< byte_t >( 'l' ),
                           static_cast< byte_t >( 'd' ),
                           static_cast< byte_t >( '!' ) }; // Hello World! //-V826
    CBufferInStream inStream{ buffer };
    UInt32 processedSize{ 0 };

    SECTION( "Reading chunks of bytes" ) {
        buffer_t result;

        SECTION( "Reading the whole buffer stream" ) {
            const size_t readSize = buffer.size();
            result.resize( readSize, static_cast< byte_t >( 0 ) );
            REQUIRE( inStream.Read( &result[ 0 ], readSize, &processedSize ) == S_OK );
            REQUIRE( processedSize == readSize );
            REQUIRE( std::memcmp( result.data(), "Hello World!", readSize ) == 0 );
        }

        SECTION( "Reading first half of the buffer stream" ) {
            const size_t readSize = buffer.size() / 2;
            result.resize( readSize, static_cast< byte_t >( 0 ) );
            REQUIRE( inStream.Read( &result[ 0 ], readSize, &processedSize ) == S_OK );
            REQUIRE( processedSize == readSize );
            REQUIRE( std::memcmp( result.data(), "Hello ", readSize ) == 0 );
        }

        SECTION( "Reading last half of the buffer stream" ) {
            const size_t readSize = buffer.size() / 2;

            UInt64 newPosition{ 0 };
            REQUIRE( inStream.Seek( readSize, STREAM_SEEK_SET, &newPosition ) == S_OK );
            REQUIRE( newPosition == readSize );

            result.resize( readSize, static_cast< byte_t >( 0 ) );
            REQUIRE( inStream.Read( &result[ 0 ], readSize, &processedSize ) == S_OK );
            REQUIRE( processedSize == readSize );
            REQUIRE( std::memcmp( result.data(), "World!", readSize ) == 0 );
        }

        SECTION( "Reading the middle part of the buffer stream" ) {
            const Int64 readOffset = static_cast< Int64 >( buffer.size() ) / 4; //-V112
            UInt64 newPosition{ 0 };
            REQUIRE( inStream.Seek( readOffset, STREAM_SEEK_SET, &newPosition ) == S_OK );
            REQUIRE( newPosition == readOffset );

            const size_t readSize = buffer.size() / 2;
            result.resize( readSize, static_cast< byte_t >( 0 ) );
            REQUIRE( inStream.Read( &result[ 0 ], readSize, &processedSize ) == S_OK );
            REQUIRE( processedSize == readSize );
            REQUIRE( std::memcmp( result.data(), "lo Wor", readSize ) == 0 );
        }

        SECTION( "Trying to read more characters than the ones in the input buffer" ) {
            result.resize( buffer.size(), static_cast< byte_t >( 0 ) );
            REQUIRE( inStream.Read( &result[ 0 ], buffer.size() * 2, &processedSize ) == S_OK );
            REQUIRE( processedSize == buffer.size() );
            REQUIRE( std::memcmp( result.data(), "Hello World!", buffer.size() ) == 0 );
        }
    }

    SECTION( "Reading single characters" ) {
        auto result = static_cast< byte_t >( 'A' ); // A character not in the buffer
        UInt64 newPosition{ 0 };

        REQUIRE( inStream.Seek( buffer.size() / 2, STREAM_SEEK_SET, &newPosition ) == S_OK );
        REQUIRE( newPosition == ( buffer.size() / 2 ) );

        REQUIRE( inStream.Read( &result, 1, &processedSize ) == S_OK );
        REQUIRE( processedSize == 1 );
        REQUIRE( result == static_cast< byte_t >( 'W' ) );

        // Seeking back to the start of the stream
        REQUIRE( inStream.Seek( 0, STREAM_SEEK_SET, &newPosition ) == S_OK );
        REQUIRE( newPosition == 0 );

        REQUIRE( inStream.Read( &result, 1, &processedSize ) == S_OK );
        REQUIRE( processedSize == 1 );
        REQUIRE( result == static_cast< byte_t >( 'H' ) );

        // Seeking to the last character of the stream
        REQUIRE( inStream.Seek( -1, STREAM_SEEK_END, &newPosition ) == S_OK );
        REQUIRE( newPosition == buffer.size() - 1 );

        REQUIRE( inStream.Read( &result, 1, &processedSize ) == S_OK );
        REQUIRE( processedSize == 1 );
        REQUIRE( result == static_cast< byte_t >( '!' ) );
    }

    SECTION( "Reading from the end of the stream" ) {
        UInt64 newPosition{ 0 };
        REQUIRE( inStream.Seek( 0, STREAM_SEEK_END, &newPosition ) == S_OK );
        REQUIRE( newPosition == buffer.size() );

        auto result = static_cast< byte_t >( 'A' ); // A character not in the buffer
        REQUIRE( inStream.Read( &result, 1, &processedSize ) == S_OK ); // Not an error,
        REQUIRE( processedSize == 0 ); // but we didn't read anything, as expected!
        REQUIRE( result == static_cast< byte_t >( 'A' ) ); // And hence, the result value was not changed!
    }

    SECTION( "Reading nothing from the stream" ) {
        auto result  = static_cast< byte_t >( 'A' ); // A character not in the buffer
        REQUIRE( inStream.Read( &result, 0, &processedSize ) == S_OK ); // Not an error,
        REQUIRE( processedSize == 0 ); // but we didn't read anything, as expected!
        REQUIRE( result == static_cast< byte_t >( 'A' ) ); // And hence, the result value was not changed!
    }
}