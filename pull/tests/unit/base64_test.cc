#include "prometheus/base64.h"

#include <gmock/gmock.h>

#include <nonstd/string_view.hpp>

namespace prometheus {
namespace {

using namespace testing;

TEST(Base64Test, Decode) {
  using namespace nonstd::string_view_literals;
  std::map<std::string, std::string> samples{
      {"Rg==", "F"},
      {"Rk8=", "FO"},
      {"Rk9P", "FOO"},
      {"0+/1", "\xd3\xef\xf5"},
      {"TWFuIGlzIGRpc3Rpbmd1aXNoZWQsIG5vdCBvbmx5IGJ5IGhpcyByZWFzb24sIGJ1dCBieSB"
       "0aGlzIHNpbmd1bGFyIHBhc3Npb24gZnJvbSBvdGhlciBhbmltYWxzLCB3aGljaCBpcyBhIG"
       "x1c3Qgb2YgdGhlIG1pbmQsIHRoYXQgYnkgYSBwZXJzZXZlcmFuY2Ugb2YgZGVsaWdodCBpb"
       "iB0aGUgY29udGludWVkIGFuZCBpbmRlZmF0aWdhYmxlIGdlbmVyYXRpb24gb2Yga25vd2xl"
       "ZGdlLCBleGNlZWRzIHRoZSBzaG9ydCB2ZWhlbWVuY2Ugb2YgYW55IGNhcm5hbCBwbGVhc3V"
       "yZS4=",
       "Man is distinguished, not only by his reason, but by this singular "
       "passion from other animals, which is a lust of the mind, that by a "
       "perseverance of delight in the continued and indefatigable generation "
       "of knowledge, exceeds the short vehemence of any carnal pleasure."}};

  for (const auto& sample : samples) {
    auto decodedBytes = base64::decode(sample.first);

    nonstd::string_view decoded{reinterpret_cast<char*>(decodedBytes.data()),
                                decodedBytes.size()};

    EXPECT_EQ(sample.second, decoded);
  }
}

}  // namespace
}  // namespace prometheus
