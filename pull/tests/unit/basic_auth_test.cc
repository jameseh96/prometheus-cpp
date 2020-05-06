#include <gmock/gmock.h>
#include <prometheus/auth/basic_auth.h>

namespace prometheus {
namespace {

class TestBasicAuthHandler : public prometheus::BasicAuthHandler {
 public:
  bool authorize(const std::string& user,
                 const std::string& password) override {
    return user == "test_user" && password == "test_password";
  }
};

using namespace testing;

TEST(BasicAuthTest, CredentialsFromBasicHeader) {
  // Test that BasicAuthHandler correctly unpacks the value of the
  // Authorization header into username and password.
  // Testing the CivetAuthHandler extracts the header correctly from a request
  // would require mg_connection, which is defined in civetweb.c and
  // does not appear to be conveniently mockable.

  TestBasicAuthHandler testHandler;
  AuthHandler* handler = &testHandler;
  // test_user:test_password
  EXPECT_TRUE(handler->authorize("Basic dGVzdF91c2VyOnRlc3RfcGFzc3dvcmQ="));
  // other_user:test_password
  EXPECT_FALSE(handler->authorize("Basic b3RoZXJfdXNlcjp0ZXN0X3Bhc3N3b3Jk"));
  // test_user:other_password
  EXPECT_FALSE(handler->authorize("Basic dGVzdF91c2VyOm90aGVyX3Bhc3N3b3Jk"));
  // other_user:other_password
  EXPECT_FALSE(
      handler->authorize("Basic b3RoZXJfdXNlcjpvdGhlcl9wYXNzd29yZA=="));
}

}  // namespace
}  // namespace prometheus
