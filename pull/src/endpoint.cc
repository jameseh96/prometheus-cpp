#include "prometheus/endpoint.h"

#include "handler.h"
#include "prometheus/auth/auth_handler.h"

namespace prometheus {

Endpoint::Endpoint(std::string uri, std::shared_ptr<AuthHandler> auth)
    : endpoint_registry_(std::make_shared<Registry>()),
      metrics_handler_(
          new detail::MetricsHandler{collectables_, *endpoint_registry_}),
      uri_(std::move(uri)),
      auth_handler_(std::move(auth)) {
  RegisterCollectable(endpoint_registry_);
}

Endpoint::~Endpoint() = default;

void Endpoint::RegisterCollectable(
    const std::weak_ptr<Collectable>& collectable) {
  collectables_.push_back(collectable);
}

detail::MetricsHandler* Endpoint::getMetricsHandler() const {
  return metrics_handler_.get();
}

AuthHandler* Endpoint::getAuthHandler() const { return auth_handler_.get(); }

const std::string& Endpoint::getURI() const { return uri_; }

}  // namespace prometheus