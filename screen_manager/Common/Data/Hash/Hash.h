#pragma once

#include <cstdlib>

namespace SCREEN_hash {

// Fairly decent function for hashing strings.
uint32_t Adler32(const uint8_t *data, size_t len);

}  // namespace SCREEN_hash

