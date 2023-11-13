#ifndef CRC_HPP
#define CRC_HPP

#include <cstddef>
#include <cstdint>

namespace bit7z { // NOLINT(*-concat-nested-namespaces)
namespace test {

// NOLINTNEXTLINE(*-easily-swappable-parameters)
auto crc32( const void* buffer, std::size_t length ) noexcept -> uint32_t;

template< typename T >
auto crc32( const T& buffer ) -> uint32_t {
    return crc32( buffer.data(), buffer.size() );
}

// NOLINTNEXTLINE(*-easily-swappable-parameters)
auto crc16( const void* buffer, std::size_t length ) noexcept -> uint16_t;  // CRC-16/LHA standard

template< typename T >
auto crc16( const T& buffer ) -> uint32_t {  // CRC-16/LHA standard
    return crc16( buffer.data(), buffer.size() );
}

} // namespace test
} // namespace bit7z

#endif //CRC_HPP
