//
// Created by rik20 on 13/06/2020.
//

#ifndef HRESULTCATEGORY_HPP
#define HRESULTCATEGORY_HPP

#include <system_error>
#include <string>

#ifdef _WIN32
#include <Windows.h>
#else
#include <myWindows/StdAfx.h>
#endif

namespace bit7z {
    struct hresult_category_t : public std::error_category {
        static_assert( sizeof( int ) >= sizeof( HRESULT ), "HRESULT type must be at least the size of int" );

        explicit hresult_category_t() = default;

        const char* name() const noexcept override;

        std::string message( int ev ) const override;

        std::error_condition default_error_condition( int ev ) const noexcept override;
    };

    std::error_category& hresult_category() noexcept;
}

#endif //HRESULTCATEGORY_HPP
