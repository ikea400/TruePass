#include "DeviceBinder.h"

#include <crypto/kmac.h>
#include <qbytearray.h>
#include <qbytearrayview.h>
#include <qcoreapplication.h>
#include <qstring.h>
#include <qsysinfo.h>
#include <utils/BinEncoding.h>
#include <utils/SecureArray.h>
#include <utils/ScopedTimer.h>

#include <cstdint>
#include <expected>
#include <memory>
#include <string>
#include <utility>

using namespace ikea400;
using namespace ikea400::crypto;

std::expected<DeviceBinderPtr, DeviceBindingError> DeviceBinder::create(
    const std::string& identifier) {
  SecureArray<uint8_t, 16> digest;
  size_t len =
      kmac::derive<false>(kmac::Variant::KMAC_128, QSysInfo::machineUniqueId(),
                          kDeviceBindingKey, identifier, digest);
  if (len != digest.size())
    return std::unexpected(DeviceBindingError::BindingFailed);

  QByteArrayView digestView(digest.data(), digest.size());

  QString companyName = QCoreApplication::organizationName();
  QString appName = QCoreApplication::applicationName();

  if (companyName.isEmpty()) companyName = "UnknownCompany";
  if (appName.isEmpty()) appName = "UnknownApp";

  // [Company].[AppName].[KeyPurpose].[UniqueId]

  QString finalKeyName = QString("%1.%2.%3.%4")
                             .arg(companyName)
                             .arg(appName)
                             .arg("TokenSigning")
                             .arg(digestView.toByteArray().toBase64());

  DeviceBinderPtr binder =
      std::make_unique<DeviceBinder>(finalKeyName.toStdWString());

  if (!binder->m_tpmProvider.isAvailable()) {
    return std::unexpected(DeviceBindingError::HardwareUnavailable);
  }

  return binder;
}

DeviceBinder::DeviceBinder(std::wstring deviceKeyName)
    : m_deviceKeyName(std::move(deviceKeyName)) {}

std::expected<std::vector<uint8_t>, DeviceBindingError>
DeviceBinder::getPublicKey() const {
  if (!m_tpmProvider.isAvailable()) {
    return std::unexpected(DeviceBindingError::HardwareUnavailable);
  }

  ikea400::ScopedTimer timer("DeviceBinder::getPublicKey");

  return m_tpmProvider.getOrCreatePublicKey(m_deviceKeyName)
      .transform_error([](TpmProvider::Error /*err*/) {
        return DeviceBindingError::PlatformError;
      });
}

std::expected<std::vector<uint8_t>, DeviceBindingError>
DeviceBinder::signChallenge(std::span<const uint8_t> challenge) {
  if (!m_tpmProvider.isAvailable()) {
    return std::unexpected(DeviceBindingError::HardwareUnavailable);
  }

  ikea400::ScopedTimer timer("DeviceBinder::signChallenge");

  return m_tpmProvider.sign(m_deviceKeyName, challenge)
      .transform_error([](TpmProvider::Error /*err*/) {
        return DeviceBindingError::BindingFailed;
      });
}
