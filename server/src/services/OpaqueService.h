#pragma once
#include <drogon/Session.h>
#include <drogon/drogon.h>
#include <opaque++.h>

#include <cstdint>
#include <iostream>
#include <memory>
#include <span>
#include <string>
#include <vector>

#include "../utils/StartupEntry.h"

namespace ikea400 {
class OpaqueContext;
class OpaqueService : public StartupEntry<OpaqueService> {
 public:
  static OpaqueService& instance() {
    static OpaqueService inst;
    return inst;
  }

  OpaqueContext createContext();

  void onBeginning() noexcept;

 private:
  OpaqueService() = default;

  friend void startup_beginning(OpaqueService*) {
    OpaqueService::instance().onBeginning();
  }

 private:
  std::shared_ptr<opaque::OpaqueServerSetup> m_serverSetup;

  const static inline std::string kSessionName = "opaque_service";
};

class OpaqueContext {
 public:
  OpaqueContext(const std::shared_ptr<opaque::OpaqueServerSetup>& serverSetup);

  void init(const std::string& clientIdentifier);

  std::vector<uint8_t> startRegistration(
      const std::string& clientIdentifier,
      const std::span<const uint8_t> registrationRequest);

  std::vector<uint8_t> finishRegistration(
      const std::span<const uint8_t> registrationData);

  std::vector<uint8_t> startLogin(
      const std::string& clientIdentifier,
      const std::span<const uint8_t> registrationRecord,
      const std::span<const uint8_t> loginRequest);

  bool finishLogin(const std::span<const uint8_t> loginRequest);

  std::span<const uint8_t> getSessionKey() const;

 private:
  enum class Step {
    None,
    Failed,
    RegistrationStarted,
    RegistrationFinished,
    LoginStarted,
    LoginFinished
  };

 private:
  std::unique_ptr<opaque::OpaqueServer> m_opaqueServer;
  std::shared_ptr<opaque::OpaqueServerSetup> m_serverSetup;
  Step m_currentStep = Step::None;
};

}  // namespace ikea400