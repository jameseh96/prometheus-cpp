#include <array>
#include <nonstd/string_view.hpp>
#include <stdexcept>
#include <string>
#include <vector>

namespace base64 {

/**
 * Base64 packs groups of 3 bytes into 4 chunks of 6-bits (sextets)
 *  * The 6 bit values (maximum value of 0b111111, 63) are used as the index
 * into an alphabet - usually:
 *
 * ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/
 *
 *        B                A               Z        ◄ input bytes
 * ┏━━━━━━━━━━━━━┓ ┏━━━━━━━━━━━━━┓ ┏━━━━━━━━━━━━━┓
 * 0 1 0 0 0 0 1 0 0 1 0 0 0 0 0 1 0 1 0 1 1 0 1 0  ◄ bits
 * ┗━━━━━━━━━┛ ┗━━━━━━━━━┛ ┗━━━━━━━━━┛ ┗━━━━━━━━━┛
 *      16          36          5           26      ◄ sextets
 *      Q           k           F           a       ◄ looked-up chars
 *
 * In the above example "BAZ" maps to "QkFa".
 *
 * If the input length is not a multiple of 3, fewer sextets will be
 * required to contain all the input bits. The output will be
 * padded with "="
 *
 *        B                A                        ◄ input bytes
 * ┏━━━━━━━━━━━━━┓ ┏━━━━━━━━━━━━━┓
 * 0 1 0 0 0 0 1 0 0 1 0 0 0 0 0 1 0 0 0 0 0 0 0 0  ◄ bits
 * ┗━━━━━━━━━┛ ┗━━━━━━━━━┛ ┗━━━━━━━━━┛ ┗━━━━━━━━━┛
 *      16          36          4           --      ◄ sextets
 *      Q           k           E           =       ◄ looked-up chars
 *
 *        B                A                        ◄ input bytes
 * ┏━━━━━━━━━━━━━┓
 * 0 1 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0  ◄ bits
 * ┗━━━━━━━━━┛ ┗━━━━━━━━━┛ ┗━━━━━━━━━┛ ┗━━━━━━━━━┛
 *      16          32          --          --      ◄ sextets
 *      Q           g           =           =       ◄ looked-up chars
 *
 *
 * This file only implements decoding - the inverse of this
 * operation.
 */

/**
 * Find the index of a char in the alphabet, to get the numeric values
 * of the packed sextets.
 *
 * Encoding uses the sextet bits to find the output character, this
 * method does the inverse.
 *
 * E.g.,
 *
 * 0 1 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0  ◄ bits
 * ┗━━━━━━━━━┛ ┗━━━━━━━━━┛ ┗━━━━━━━━━┛ ┗━━━━━━━━━┛
 *      16          32          --          --      ◄ sextets
 *      Q           g           =           =       ◄ looked-up chars
 *
 * charToSextet('Q') == 16
 */
uint8_t charToSextet(char input) {
  /* Equivalent to
   *  std::string alphabet =
   *      "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
   *  return alphabet.find(input);
   */
  if (input >= 'A' && input <= 'Z') {
    // 'A'-'Z', 65-90
    return input - 'A';
  }
  if (input >= 'a' && input <= 'z') {
    // 'a'-'z', 97-122
    return 26 + input - 'a';
  }
  if (input >= '0' && input <= '9') {
    // '0'-'9', 48-57
    return 52 + input - '0';
  }
  if (input == '+') {
    return 62;
  }
  if (input == '/') {
    return 63;
  }

  throw std::runtime_error(std::string("Base64 unexpected character :'") +
                           input + "'");
}

/**
 * Maps at most 4 chars which are in the base64 alphabet back to sextets, and
 * packs them into the high 24 bits of a uint32_t.
 *
 * E.g.,
 *
 * 0 1 0 0 0 0 1 0 0 1 0 0 0 0 0 1 0 1 0 1 1 0 1 0  ◄ bits
 * ┗━━━━━━━━━┛ ┗━━━━━━━━━┛ ┗━━━━━━━━━┛ ┗━━━━━━━━━┛
 *      16          36          5           26      ◄ sextets
 *      Q           k           F           a       ◄ looked-up chars
 *
 * packCharsAsSextets("QkFa") == 0b010000100100000101011010 00000000
 */
uint32_t packCharsAsSextets(nonstd::string_view input) {
  if (input.size() < 2 || input.size() > 4) {
    throw std::logic_error("Base64 packing wrong number of sextets:" +
                           std::to_string(input.size()));
  }
  uint32_t packed = 0;
  for (const auto& c : input) {
    packed <<= 6u;
    packed |= charToSextet(c);
  }

  // shift packed sextets so they start at the highest bit, for convenience
  // when unpacking
  packed <<= 32 - (input.size() * 6);
  return packed;
}

/**
 * Unpacks the high 24 bits of a uint32_t as 3 bytes
 *
 *        B                A               Z        ◄ input bytes
 * ┏━━━━━━━━━━━━━┓ ┏━━━━━━━━━━━━━┓ ┏━━━━━━━━━━━━━┓
 * 0 1 0 0 0 0 1 0 0 1 0 0 0 0 0 1 0 1 0 1 1 0 1 0  ◄ bits
 */
void unpackOctets(uint32_t packed, size_t octetCount,
                  std::vector<uint8_t>& output) {
  if (octetCount == 0 || octetCount > 3) {
    throw std::logic_error("Base64 unpacking wrong number of octets:" +
                           std::to_string(octetCount));
  }

  // Could also be implemented with htonl and reinterpret_cast
  // done this way to mirror the sextet packing.
  for (size_t i = 0; i < octetCount; i++) {
    // read the high byte
    output.emplace_back((packed & 0xFF000000u) >> 24u);
    // shift the high byte out
    packed <<= 8u;
  }
}

/**
 * Decode at most 4 characters of a base64 encoded string as bytes.
 */
void decodeChars(nonstd::string_view input, std::vector<uint8_t>& output) {
  auto packed = packCharsAsSextets(input);
  /**
   * in the valid range of input lengths (2, 3, 4) there will be one fewer
   * octet to unpack than sextets packed. See the earlier diagram for each case.
   * Padding should be stripped before calling.
   */

  unpackOctets(packed, input.size() - 1, output);
}

/**
 * Decode a valid base64 encoded string back to bytes.
 * Input must be a multiple of 4 characters long (possibly
 * padded).
 */
std::vector<uint8_t> decode(nonstd::string_view input) {
  if (input.empty()) {
    return {};
  }
  if (input.size() % 4 != 0) {
    throw std::runtime_error("Base64 input length not a multiple of 4");
  }

  std::vector<uint8_t> output;

  // decoded output will be shorter than the input. Reserve 3/4 of
  // the input length to reduce reallocations (ignoring padding, 4 input
  // characters maps to 3 output bytes).
  output.reserve(input.size() * 0.75);

  auto paddingCount = 0;

  if (input.ends_with("==")) {
    paddingCount = 2;
  } else if (input.ends_with('=')) {
    paddingCount = 1;
  }

  for (size_t i = 0; i < input.size(); i += 4) {
    if (i + 4 < input.size()) {
      // not at the end, don't need to account for padding
      decodeChars(input.substr(i, 4), output);
    } else {
      // final group of input characters, which may contain zero to two
      // characters of '=' padding. '=' is not in the alphabet, and
      // does not need to be decoded.
      decodeChars(input.substr(i, 4 - paddingCount), output);
    }
  }

  return output;
}

}  // namespace base64