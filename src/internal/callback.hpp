/*
 * bit7z - A C++ static library to interface with the 7-zip shared libraries.
 * Copyright (c) Riccardo Ostani - All Rights Reserved.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef CALLBACK_HPP
#define CALLBACK_HPP

#if !defined( _MSC_VER ) || defined( __clang__ )
#include "bitdefines.hpp"
#endif

#include "bitexception.hpp"
#include "internal/com.hpp"
#include "internal/exceptionutil.hpp"
#include "internal/guids.hpp"

#include <exception>
#include <new>
#include <system_error>
#include <utility>

namespace bit7z {

/* On Windows, 7-zip's CMyUnknownImp class has a virtual destructor if the compiler is MinGW/GCC/Clang.
 * On MSVC or on Unix, the destructor is not virtual. */
#ifdef _WIN32
#   if ( defined(__GNUC__) || defined(__clang__) ) && !defined( SEVENZIP_2301 )
#       define CALLBACK_DESTRUCTOR( x ) x override
#   endif
#endif

#ifndef CALLBACK_DESTRUCTOR // MSVC or Unix compiler
#   define CALLBACK_DESTRUCTOR( x ) virtual x
#endif

/* MSVC's backend refuses to honor __forceinline on guardOperation() (below), emitting C4714 (fatal under /WX)
 * because of its multiple catch handlers; GCC/Clang (including clang-cl, which also defines _MSC_VER) inline it fine,
 * so only genuine cl.exe skips the attempt. */
#if defined( _MSC_VER ) && !defined( __clang__ )
#   define BIT7Z_ALWAYS_INLINE_GUARD
#else
#   define BIT7Z_ALWAYS_INLINE_GUARD BIT7Z_ALWAYS_INLINE
#endif

class BitAbstractArchiveHandler;

class Callback : protected CMyUnknownImp {
    public:
        Callback( const Callback& ) = delete;

        Callback( Callback&& ) = delete;

        auto operator=( const Callback& ) -> Callback& = delete;

        auto operator=( Callback&& ) -> Callback& = delete;

        CALLBACK_DESTRUCTOR( ~Callback() ) = default;

        /**
         * If an exception was stored (via setErrorException()/guardOperation()), rethrows it and never
         * returns; otherwise does nothing. Called by the driving loop (BitInputArchive::extractArchive(),
         * BitOutputArchive::compressOut()) once the 7-Zip call that invoked this callback returns.
         */
        void rethrowStoredException() const;

    protected:
        explicit Callback( const BitAbstractArchiveHandler& handler ); // Abstract class

        // Constructs a BitException from message and code and stores it as the last error.
        void setErrorException( const char* message, std::error_code code ) noexcept;

        // Runs `operation`, catching whatever it throws (storing it via setErrorException(), above)
        // so it can't escape a noexcept COM method such as GetStream().
        template< typename Operation >
        BIT7Z_ALWAYS_INLINE_GUARD
        auto guardOperation( Operation&& operation ) noexcept -> HRESULT;

        const BitAbstractArchiveHandler& mHandler;

    private:
        // Wraps exception via std::make_exception_ptr() and stores it as the last error.
        void setErrorException( const BitException& exception ) noexcept;

        std::exception_ptr mErrorException;
};

template< typename Operation >
BIT7Z_ALWAYS_INLINE_GUARD
auto Callback::guardOperation( Operation&& operation ) noexcept -> HRESULT {
    try {
        return std::forward< Operation >( operation )();
    } catch ( const BitException& exception ) {
        setErrorException( exception );
        return exception.hresultCode();
    } catch ( const std::system_error& exception ) {
        // exception.what() already embeds its category's message, and BitException's own
        // std::system_error base would append that same message again if reused here, so only the
        // error code is preserved, not the original message.
        setErrorException( "Failed to get the stream", exception.code() );
        return E_ABORT;
    } catch ( const std::bad_alloc& ) {
        // Avoid allocating while already handling an out-of-memory condition: unlike toBitException()
        // (which needs a new string + BitException), current_exception() only bumps a refcount on the
        // exception object the runtime already allocated when it was thrown.
        mErrorException = std::current_exception();
        return E_OUTOFMEMORY;
    } catch ( const std::exception& exception ) {
        setErrorException( toBitException( "Failed to get the stream", exception ) );
        return E_ABORT;
    } catch ( ... ) {
        // E.g., a user-provided callback threw an exception not derived from std::exception; it must
        // not escape this noexcept COM method, so it's stored for the driving loop to rethrow.
        mErrorException = std::current_exception();
        return E_ABORT;
    }
}

} // namespace bit7z

#endif // CALLBACK_HPP
