#pragma once

#include <nonstd/string_view.hpp>
#include <string_view>
#include <vector>

namespace base64 {

/**
 * Decode a valid base64 encoded string back to bytes.
 * Input must be a multiple of 4 characters long (possibly
 * padded).
 *
 * Newlines and whitespace are not ignored and will cause decoding
 * to fail; they should be removed ahead of time.
 */
std::vector<uint8_t> decode(nonstd::string_view input);

}  // namespace base64