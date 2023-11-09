#ifndef CRC_HPP
#define CRC_HPP

#include <cstddef>
#include <cstdint>

namespace bit7z { // NOLINT(*-concat-nested-namespaces)
namespace test {

// NOLINTNEXTLINE(*-easily-swappable-parameters)
auto crc32( const void* buffer, std::size_t length, uint32_t initial ) -> uint32_t;

template< typename T >
auto crc32( const T& buffer, uint32_t initial = 0 ) -> uint32_t {
    return crc32( buffer.data(), buffer.size(), initial );
}

} // namespace test
} // namespace bit7z

#endif //CRC_HPP
