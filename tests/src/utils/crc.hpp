#ifndef CRC_HPP
#define CRC_HPP

#include <bit7z/bittypes.hpp>

#include <cstddef>
#include <cstdint>

namespace bit7z {
namespace test {
auto crc32( const buffer_t& buffer, uint32_t initial = 0 ) -> uint32_t;
} // namespace test
} // namespace bit7z

#endif //CRC_HPP
