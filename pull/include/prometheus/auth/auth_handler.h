#pragma once

#include <string>
#include <unordered_map>

#include "prometheus/detail/pull_export.h"

class CivetServer;
class CivetAuthHandler;

namespace prometheus {

namespace detail {
class ForwardingCivetAuthHandler;
}  // namespace detail

/**
 * Handler for simple header-based authentication/authorization of Endpoints.
 *
 * Should be derived from from to implement specific auth methods
 * e.g., Basic, Digest, Bearer.
 */
class PROMETHEUS_CPP_PULL_EXPORT AuthHandler {
 public:
  AuthHandler();
  virtual ~AuthHandler();

  /**
   * Called for every incoming scrape request. If this returns
   * false, the request will be rejected and will not be passed to
   * a MetricHandler.
   *
   * @param auth raw value of the HTTP Authorization header
   * @return true to accept, false to reject the request
   */
  virtual bool authorize(const std::string& auth) = 0;

  /**
   * Get the underlying CivetAuthHandler. Used to register the handler
   * with civetweb.
   */
  CivetAuthHandler* getCivetAuthHandler() const;

 protected:
  /**
   * Add a header to be written as part of every Unauthorized response.
   *
   * Could be used to set, for example, the WWW-Authenticate header challenge
   * if implementing Basic or Digest Auth.
   */
  void addUnauthorizedHeader(const std::string& key, const std::string& value);

  // This CivetAuthHandler will be registered with a CivetServer, and will
  // call back to AuthHandler::authorize().
  // PIMPL-like composition avoids users needing CivetServer.h or having to
  // interact with mg_connection directly.
  std::unique_ptr<detail::ForwardingCivetAuthHandler> civetAuthHandler;
};

}  // namespace prometheus