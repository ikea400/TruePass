#include "OpaqueService.h"

#include <drogon/HttpAppFramework.h>
#include <drogon/Session.h>
#include <json/value.h>
#include <opaque++.h>
#include <trantor/utils/Logger.h>
#include <utils/BinEncoding.h>

#include <cstdint>
#include <exception>
#include <memory>
#include <span>
#include <stdexcept>
#include <string>
#include <vector>

using namespace ikea400;

using namespace opaque;

OpaqueContext ikea400::OpaqueService::createContext() {
  return OpaqueContext{m_serverSetup};
}

void OpaqueService::onBeginning() noexcept {
  const auto& config = drogon::app().getCustomConfig();

  const Json::Value* opaqueSetup = config.find("opaque_setup");
  if (!opaqueSetup || !opaqueSetup->isString()) {
    LOG_FATAL << "OpaqueService::onBeginning Opaque setup not found in config";
    return;
  }

  try {
    std::vector<uint8_t> decodedSetup =
        ikea400::bin::decodeFormat(opaqueSetup->asString());

    m_serverSetup = std::make_shared<OpaqueServerSetup>(decodedSetup);

    LOG_INFO << "Loaded Opaque server setup with static public key: "
             << bin::hex::encode(opaque::OpaqueServerSetup(decodedSetup)
                                     .getStaticPublicKey());

  } catch (const std::exception& e) {
    LOG_FATAL
        << "OpaqueService::onBeginning Failed to initialize Opaque server "
           "setup: "
        << e.what();
  }
}

OpaqueContext::OpaqueContext(
    const std::shared_ptr<opaque::OpaqueServerSetup>& serverSetup)
    : m_serverSetup(serverSetup) {}

void OpaqueContext::init(const std::string& clientIdentifier) {
  m_opaqueServer =
      std::make_unique<OpaqueServer>(m_serverSetup, clientIdentifier, "", "");

  m_currentStep = Step::None;
}

std::vector<uint8_t> OpaqueContext::startRegistration(
    const std::string& clientIdentifier,
    const std::span<const uint8_t> registrationRequest) {
  init(clientIdentifier);

  try {
    std::vector<uint8_t> response =
        m_opaqueServer->startRegistration(registrationRequest);
    m_currentStep = Step::RegistrationStarted;
    return response;
  } catch (...) {
    m_currentStep = Step::Failed;
    return {};
  }
}

std::vector<uint8_t> OpaqueContext::finishRegistration(
    const std::span<const uint8_t> registrationData) {
  if (!m_opaqueServer || m_currentStep != Step::RegistrationStarted) {
    m_currentStep = Step::Failed;
    throw std::runtime_error(
        "OpaqueContext::finishRegistration Invalid step: Registration not "
        "started or already completed");
  }

  try {
    std::vector<uint8_t> passwordFile =
        m_opaqueServer->finishRegistration(registrationData);
    m_currentStep = Step::RegistrationFinished;
    return passwordFile;
  } catch (...) {
    m_currentStep = Step::Failed;
    return {};
  }
}

std::vector<uint8_t> ikea400::OpaqueContext::startLogin(
    const std::string& clientIdentifier,
    const std::span<const uint8_t> registrationRecord,
    const std::span<const uint8_t> loginRequest) {
  init(clientIdentifier);

  try {
    std::vector<uint8_t> response = m_opaqueServer->startLogin(
        clientIdentifier, registrationRecord, loginRequest);
    m_currentStep = Step::LoginStarted;
    return response;
  } catch (...) {
    m_currentStep = Step::Failed;
    return {};
  }
}

bool ikea400::OpaqueContext::finishLogin(
    const std::span<const uint8_t> loginRequest) {
  if (!m_opaqueServer || m_currentStep != Step::LoginStarted) {
    throw std::runtime_error(
        "OpaqueContext::finishLogin Invalid step: Login not started or already "
        "completed");
    return false;
  }

  try {
    m_opaqueServer->finishLogin(loginRequest);
    m_currentStep = Step::LoginFinished;
    return true;
  } catch (...) {
    m_currentStep = Step::Failed;
    return false;
  }
}

std::span<const uint8_t> ikea400::OpaqueContext::getSessionKey() const {
  if (m_currentStep != Step::LoginFinished) {
    throw std::runtime_error(
        "OpaqueContext::getSessionKey Invalid step: Login not finished");
  }
  return m_opaqueServer->getSessionKey();
}
