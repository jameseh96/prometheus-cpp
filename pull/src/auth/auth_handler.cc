#include "prometheus/auth/auth_handler.h"

#include "CivetServer.h"
#include "prometheus/detail/future_std.h"

namespace prometheus {

namespace detail {

/**
 * CivetAuthHander which simply forwards Auth challenges to an AuthHandler
 * instance. Used to avoid requiring users to include (and be aware of) civetweb
 * related headers. Should only be created by an AuthHandler.
 */
class ForwardingCivetAuthHandler : public CivetAuthHandler {
 public:
  explicit ForwardingCivetAuthHandler(AuthHandler& handler)
      : handler(handler) {}

  /**
   * Called by CivetWeb to authorize a request.
   *
   * Forwards the "Authorization" header on to the owning
   * AuthHandler, `this->handler`.
   * If handler returns true, permits the request to proceed.
   * If handler returns false, or the Auth header is absent,
   * rejects the request with 401 Unauthorized.
   */
  bool authorize(CivetServer* server, mg_connection* conn) override {
    const char* authHeader = mg_get_header(conn, "Authorization");

    if (authHeader == nullptr) {
      // No auth header was provided. Will respond with 401 and any
      // unauthorizedHeaders which have been set e.g., WWW-Authenticate
      writeUnauthorizedResponse(conn);
      return false;
    }

    bool success = handler.authorize(authHeader);
    if (!success) {
      // TODO: bool does not permit a distinction between 401 Unauthorized
      //  and 403 Forbidden. Authentication may succeed, but the user still
      //  not be authorized to perform the request.
      writeUnauthorizedResponse(conn);
      return false;
    }
    return true;
  };

  void addUnauthorizedHeader(const std::string& key, const std::string& value) {
    unauthorizedHeaders.emplace(key, value);
  }

 private:
  void writeUnauthorizedResponse(mg_connection* conn) {
    mg_printf(conn, "HTTP/1.1 401 Unauthorized\r\n");

    for (const auto& header : unauthorizedHeaders) {
      mg_printf(conn, "%s: %s\r\n", header.first.c_str(),
                header.second.c_str());
    }
    // end headers
    mg_printf(conn, "\r\n");
  }

  AuthHandler& handler;
  std::unordered_map<std::string, std::string> unauthorizedHeaders;
};

}  // namespace detail

AuthHandler::AuthHandler()
    : civetAuthHandler(
          detail::make_unique<detail::ForwardingCivetAuthHandler>(*this)) {}

// defined here as destruction requires DelegatingCovetAuthHandler be
// a complete type (PIMPL)
AuthHandler::~AuthHandler() = default;

CivetAuthHandler* AuthHandler::getCivetAuthHandler() const {
  return civetAuthHandler.get();
}

void AuthHandler::addUnauthorizedHeader(const std::string& key,
                                        const std::string& value) {
  civetAuthHandler->addUnauthorizedHeader(key, value);
}

}  // namespace prometheus