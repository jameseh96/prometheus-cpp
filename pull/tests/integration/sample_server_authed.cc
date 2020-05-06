#include <chrono>
#include <memory>
#include <thread>

#include <prometheus/auth/basic_auth.h>
#include <prometheus/counter.h>
#include <prometheus/endpoint.h>
#include <prometheus/exposer.h>
#include <prometheus/registry.h>

class TestBasicAuthHandler : public prometheus::BasicAuthHandler {
 public:
  bool authorize(const std::string& user,
                 const std::string& password) override {
    return user == "test_user" && password == "test_password";
  }
};

int main() {
  using namespace prometheus;

  auto auth = std::make_shared<TestBasicAuthHandler>();

  auto endpointA = std::make_shared<Endpoint>("/metricsA", auth);
  auto endpointB = std::make_shared<Endpoint>("/metricsB", auth);

  // create a metrics registry with component=main labels applied to all its
  // metrics
  auto registryA = std::make_shared<Registry>();

  // add a new counter family to the registry (families combine values with the
  // same name, but distinct label dimensions)
  auto& counter_familyA = BuildCounter()
                              .Name("time_running_seconds_total")
                              .Help("How many seconds is this server running?")
                              .Labels({{"label", "foo"}})
                              .Register(*registryA);

  // add a counter to the metric family
  auto& seconds_counter = counter_familyA.Add(
      {{"another_label", "bar"}, {"yet_another_label", "baz"}});

  // ask the exposer to scrape registryA on incoming scrapes for "/metricsA"
  endpointA->RegisterCollectable(registryA);

  auto registryB = std::make_shared<Registry>();

  auto& counter_familyB = BuildCounter()
                              .Name("other_sent_bytes_total")
                              .Help("How many bytes has some other thing sent?")
                              .Labels({{"label", "not_foo"}})
                              .Register(*registryB);

  auto& counterB = counter_familyB.Add(
      {{"another_label", "not_bar"}, {"yet_another_label", "not_baz"}});

  // This endpoint exposes registryB.
  endpointB->RegisterCollectable(registryB);

  // create an http server running on port 8080
  MultiExposer exposer{"127.0.0.1:8080", {endpointA, endpointB}, 1};

  for (;;) {
    std::this_thread::sleep_for(std::chrono::seconds(1));
    // increment the counter by one (second)
    seconds_counter.Increment();
  }
  return 0;
}
