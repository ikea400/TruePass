#pragma once

#include <QIcon>
#include <QNetworkAccessManager>
#include <QObject>
#include <string>
#include <string_view>
#include <unordered_map>

class FaviconManager : public QObject {
  Q_OBJECT
 public:
  static FaviconManager* instance() {
    static FaviconManager instance;
    return &instance;
  }

  FaviconManager(QObject* parent = nullptr);

  QIcon getFaviconForUrl(const std::string& url);

 signals:
  void iconUpdated(const QString& domain);

 public slots:

 private:
  QIcon getCachedIcon(const std::string& domain);
  void loadIconForDomain(const std::string& domain);

  void fetchIcon(const QString& domain, int attempt = 0);

  static QString getIconsCacheDirPath();
  static QString getIconCacheFilePath(const std::string& domain);
  static QString getFileNameForDomain(const std::string& domain);

 private:
  std::unordered_map<std::string, QIcon> m_faviconCache;
  QNetworkAccessManager m_networkManager;
  QIcon m_defaultFavicon;

  const QVector<QString> m_providers = {
      "https://icons.duckduckgo.com/ip3/%1.ico",
      "https://www.google.com/s2/favicons?domain=%1&sz=64",
      "https://icon.horse/icon/%1", "https://%1/favicon.ico"};

  static inline const QString kIconDirectory = "icons";
};