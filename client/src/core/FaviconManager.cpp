#include "FaviconManager.h"

#include <crypto/hash_algo.h>
#include <crypto/hasher.h>
#include <qdir.h>
#include <qicon.h>
#include <qobject.h>
#include <qstandardpaths.h>
#include <qstring.h>
#include <utils/BinEncoding.h>
#include <utils/utils.h>

#include <QDir>
#include <QNetworkReply>
#include <QStandardPaths>
#include <array>
#include <cstdint>
#include <string>

using namespace ikea400;

FaviconManager::FaviconManager(QObject* parent)
    : QObject(parent), m_defaultFavicon(":/icons/icons/login.svg") {}

QIcon FaviconManager::getFaviconForUrl(const std::string& url) {
  std::string domain = utils::extractDomain(url);
  if (domain.empty()) return m_defaultFavicon;

  QIcon icon = getCachedIcon(domain);
  if (!icon.isNull()) {
    return icon;
  }

  loadIconForDomain(domain);

  return m_defaultFavicon;
}

QIcon FaviconManager::getCachedIcon(const std::string& domain) {
  if (auto it = m_faviconCache.find(domain); it != m_faviconCache.end()) {
    const QIcon& icon = it->second;
    if (icon.isNull()) {
      return m_defaultFavicon;
    }
    return icon;
  }

  QString cacheFilePath = getIconCacheFilePath(domain);
  if (cacheFilePath.isEmpty()) return {};

  QIcon icon(cacheFilePath);
  if (icon.isNull()) return {};

  m_faviconCache[domain] = icon;
  return icon;
}

void FaviconManager::loadIconForDomain(const std::string& domain) {
  // Mark the domain as being loaded to prevent multiple loads for
  // the same domain
  m_faviconCache[domain] = {};

  qDebug() << "Loading favicon for domain:" << QString::fromStdString(domain);

  fetchIcon(QString::fromStdString(domain));
}

void FaviconManager::fetchIcon(const QString& domain, int attempt) {
  if (attempt >= m_providers.size()) {
    qDebug() << "All providers failed for domain:" << domain;
    return;
  }

  QUrl url(m_providers[attempt].arg(domain));
  QNetworkRequest request(url);

  QNetworkReply* reply = m_networkManager.get(request);

  connect(
      reply, &QNetworkReply::finished, this, [this, reply, domain, attempt]() {
        reply->deleteLater();

        if (reply->error() == QNetworkReply::NoError) {
          QByteArray data = reply->readAll();
          QPixmap pixmap;
          if (pixmap.loadFromData(data)) {
            QIcon icon(pixmap);
            m_faviconCache[domain.toStdString()] = icon;
            QString cacheFilePath = getIconCacheFilePath(domain.toStdString());
            if (!cacheFilePath.isEmpty()) {
              pixmap.save(cacheFilePath);
            }
            emit iconUpdated(domain);
            return;
          }
        }
        this->fetchIcon(domain, attempt + 1);
      });
}

QString FaviconManager::getIconsCacheDirPath() {
  QString cacheDir =
      QStandardPaths::writableLocation(QStandardPaths::CacheLocation);
  if (cacheDir.isEmpty()) return {};

  QDir dir(cacheDir);
  if (!dir.exists(kIconDirectory)) {
    if (!dir.mkpath(kIconDirectory)) return {};
  }

  return dir.filePath(kIconDirectory);
}

QString FaviconManager::getIconCacheFilePath(const std::string& domain) {
  QString dirPath = getIconsCacheDirPath();

  QDir dir(dirPath);
  if (!dir.exists()) return {};

  QString fileName = getFileNameForDomain(domain);
  if (fileName.isEmpty()) return {};

  return dir.filePath(fileName);
}

QString FaviconManager::getFileNameForDomain(const std::string& domain) {
  using namespace crypto;

  QString domainLower = QString::fromStdString(domain).toLower();

  Hasher hasher;
  if (!hasher.init<false>(HashAlgo::Sha256)) {
    return {};
  }

  std::array<uint8_t, SHA256_HASH_SIZE> digest;
  if (!hasher.hash<false>(domainLower.toUtf8(), digest)) {
    return {};
  }

  return QString::fromStdString(bin::hex::encode(digest)) + ".png";
}
