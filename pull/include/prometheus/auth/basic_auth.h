#pragma once

#include <string>

#include "prometheus/auth/auth_handler.h"
#include "prometheus/detail/pull_export.h"

namespace prometheus {

static const std::string defaultRealm = "Prometheus-cpp Exporter";

/**
 * Auth handler implementing HTTP Basic Auth.
 */
class PROMETHEUS_CPP_PULL_EXPORT BasicAuthHandler : public AuthHandler {
 public:
  /**
   * Construct an auth handler implementing Basic HTTP Auth.
   * @param realm the realm auth s
   */
  explicit BasicAuthHandler(const std::string& realm = defaultRealm);
  /**
   * Parses a HTTP Basic Auth header into username and password.
   *
   * If this succeeds, calls authorize(username, password);
   */
  bool authorize(const std::string& authHeader) override;

  /**
   * Called with username and password for every incoming request
   * with a well-formed HTTP Basic Auth header.
   *
   * Can return true to allow the request to proceed, or false
   * to reject the request
   *
   * @param user provided username
   * @param password provided password
   * @return if the request should be accepted
   */
  virtual bool authorize(const std::string& user,
                         const std::string& password) = 0;
};

}  // namespace prometheus