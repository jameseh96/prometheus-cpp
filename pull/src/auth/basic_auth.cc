#include "prometheus/auth/basic_auth.h"

#include <nonstd/string_view.hpp>

#include "CivetServer.h"
#include "prometheus/base64.h"

namespace prometheus {

BasicAuthHandler::BasicAuthHandler(const std::string& realm) : AuthHandler() {
  std::string authHeader = "Basic realm=\"";
  authHeader += realm;
  authHeader += "\"";
  // set header sent with 401 responses
  addUnauthorizedHeader("WWW-Authenticate", authHeader);
}

bool BasicAuthHandler::authorize(const std::string& authHeaderStr) {
  // Basic auth header is expected to be of the form:
  // "Basic dXNlcm5hbWU6cGFzc3dvcmQ="
  using namespace nonstd::string_view_literals;
  // manipulate as string_view to avoid copying parts of the header
  // and for .starts_with()
  nonstd::string_view authHeader{authHeaderStr};

  const auto prefix = "Basic "_sv;
  if (!authHeader.starts_with(prefix)) {
    return false;
  }

  // Strip the "Basic " prefix leaving the base64 encoded auth string
  auto b64Auth = authHeader.substr(prefix.size());

  std::vector<uint8_t> decodedBytes;
  try {
    decodedBytes = base64::decode(b64Auth);
  } catch (...) {
    return false;
  }

  nonstd::string_view decoded{
      reinterpret_cast<const char*>(decodedBytes.data()), decodedBytes.size()};

  // decoded auth string is expected to be of the form:
  // "username:password"
  // colons may not appear in the username.
  auto splitPos = decoded.find(':');
  if (splitPos == nonstd::string_view::npos) {
    return false;
  }

  auto username = decoded.substr(0, splitPos);
  auto password = decoded.substr(splitPos + 1);

  return authorize(std::string(username), std::string(password));
}

}  // namespace prometheus