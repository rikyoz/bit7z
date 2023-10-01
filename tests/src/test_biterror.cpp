// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

/*
 * bit7z - A C++ static library to interface with the 7-zip shared libraries.
 * Copyright (c) 2014-2023 Riccardo Ostani - All Rights Reserved.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include <catch2/catch.hpp>

#include <bit7z/biterror.hpp>

#ifndef BIT7Z_TESTS_PUBLIC_API_ONLY
#include "internal/extractcallback.hpp"
#endif

using bit7z::BitError;
using bit7z::BitFailureSource;

struct ErrorSourceMapping {
    const char* errorName;
    BitError error;
    const char* sourceName;
    BitFailureSource source;
};

#define ERROR_SOURCE( error, source ) \
    ErrorSourceMapping{ "BitError::" #error, BitError::error, "BitFailureSource::" #source, BitFailureSource::source }

TEST_CASE( "BitError: Checking that error values correspond to the correct failure sources", "[BitError]" ) {
    const auto errorSource = GENERATE( as< ErrorSourceMapping >(),
                                       ERROR_SOURCE( FilterNotSpecified, InvalidArgument ),
                                       ERROR_SOURCE( FormatFeatureNotSupported, InvalidArgument ),
                                       ERROR_SOURCE( IndicesNotSpecified, InvalidArgument ),
                                       ERROR_SOURCE( InvalidArchivePath, InvalidArgument ),
                                       ERROR_SOURCE( InvalidOutputBufferSize, InvalidArgument ),
                                       ERROR_SOURCE( InvalidCompressionMethod, InvalidArgument ),
                                       ERROR_SOURCE( InvalidDictionarySize, InvalidArgument ),
                                       ERROR_SOURCE( InvalidIndex, InvalidArgument ),
                                       ERROR_SOURCE( InvalidWordSize, InvalidArgument ),
                                       ERROR_SOURCE( ItemIsAFolder, InvalidArgument ),
                                       ERROR_SOURCE( ItemMarkedAsDeleted, OperationNotPermitted ),
                                       ERROR_SOURCE( NoMatchingItems, NoSuchItem ),
                                       ERROR_SOURCE( NoMatchingSignature, InvalidArchive ),
                                       ERROR_SOURCE( NonEmptyOutputBuffer, InvalidArgument ),
                                       ERROR_SOURCE( RequestedWrongVariantType, OperationNotSupported ),
                                       ERROR_SOURCE( UnsupportedOperation, OperationNotSupported ),
                                       ERROR_SOURCE( UnsupportedVariantType, OperationNotSupported ),
                                       ERROR_SOURCE( WrongUpdateMode, OperationNotPermitted ),
                                       ERROR_SOURCE( InvalidZipPassword, InvalidArgument ) );

    DYNAMIC_SECTION( errorSource.errorName << " vs " << errorSource.sourceName ) {
        const auto errorCode = make_error_code( errorSource.error );
        REQUIRE( errorCode == errorSource.source );
    }
}

#ifndef BIT7Z_TESTS_PUBLIC_API_ONLY
using bit7z::OperationResult;

struct ResultSourceMapping {
    const char* errorName;
    OperationResult error;
    const char* sourceName;
    BitFailureSource source;
};

#define RESULT_SOURCE( result, source ) \
    ResultSourceMapping{ "OperationResult::" #result, OperationResult::result, \
                         "BitFailureSource::" #source, BitFailureSource::source }

TEST_CASE( "OperationResult: Checking that result values correspond to the correct failures", "[OperationResult]" ) {
    const auto errorSource = GENERATE( as< ResultSourceMapping >(),
                                       RESULT_SOURCE( CRCError, CRCError ),
                                       RESULT_SOURCE( CRCErrorEncrypted, CRCError ),
                                       RESULT_SOURCE( CRCErrorEncrypted, WrongPassword ),
                                       RESULT_SOURCE( DataAfterEnd, DataAfterEnd ),
                                       RESULT_SOURCE( DataError, DataError ),
                                       RESULT_SOURCE( DataErrorEncrypted, DataError ),
                                       RESULT_SOURCE( DataErrorEncrypted, WrongPassword ),
                                       RESULT_SOURCE( EmptyPassword, WrongPassword ),
                                       RESULT_SOURCE( HeadersError, HeadersError ),
                                       RESULT_SOURCE( IsNotArc, InvalidArchive ),
                                       RESULT_SOURCE( OpenErrorEncrypted, WrongPassword ),
                                       RESULT_SOURCE( Unavailable, UnavailableData ),
                                       RESULT_SOURCE( UnexpectedEnd, UnexpectedEnd ),
                                       RESULT_SOURCE( WrongPassword, WrongPassword ) );

    DYNAMIC_SECTION( errorSource.errorName << " vs " << errorSource.sourceName ) {
        const auto errorCode = make_error_code( errorSource.error );
        REQUIRE( errorCode == errorSource.source );
    }
}
#endif