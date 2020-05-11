#pragma once

#include <string>

#include "prometheus/collectable.h"
#include "prometheus/detail/pull_export.h"
#include "prometheus/registry.h"

namespace prometheus {

namespace detail {
class MetricsHandler;
}  // namespace detail
class AuthHandler;

class PROMETHEUS_CPP_PULL_EXPORT Endpoint {
 public:
  explicit Endpoint(std::string uri, std::shared_ptr<AuthHandler> auth = {});
  ~Endpoint();

  void RegisterCollectable(const std::weak_ptr<Collectable>& collectable);

  detail::MetricsHandler* getMetricsHandler() const;

  AuthHandler* getAuthHandler() const;

  const std::string& getURI() const;

 private:
  std::vector<std::weak_ptr<Collectable>> collectables_;
  // registry for "meta" metrics about the endpoint itself
  std::shared_ptr<Registry> endpoint_registry_;
  std::unique_ptr<detail::MetricsHandler> metrics_handler_;
  std::string uri_;
  std::shared_ptr<AuthHandler> auth_handler_;
};

}  // namespace prometheus