#pragma once
#include <QIcon>
#include <QString>

#include "../dto/VaultItemMetadataDto.h"

class VaultItemSummaryModel {
 public:
  VaultItemSummaryModel(const QString& name, const QString& id,
                        const QString& customIconStr, const QIcon& icon,
                        ikea400::dto::VaultItemType type, bool isFavorite,
                        bool isDeleted)
      : m_name(name),
        m_id(id),
        m_customIconStr(customIconStr),
        m_icon(icon),
        m_type(type),
        m_isFavorite(isFavorite),
        m_isDeleted(isDeleted) {}

  VaultItemSummaryModel(const std::string& name, const std::string& id,
                        const std::string& customIconStr, const QIcon& icon,
                        ikea400::dto::VaultItemType type, bool isFavorite,
                        bool isDeleted)
      : m_name(QString::fromStdString(name)),
        m_id(QString::fromStdString(id)),
        m_customIconStr(QString::fromStdString(customIconStr)),
        m_icon(icon),
        m_type(type),
        m_isFavorite(isFavorite),
        m_isDeleted(isDeleted) {}

  [[nodiscard]] inline const auto& getName() const noexcept { return m_name; }
  [[nodiscard]] inline const auto& getId() const noexcept { return m_id; }
  [[nodiscard]] inline const auto& getCustomIconStr() const noexcept {
    return m_customIconStr;
  }
  [[nodiscard]] inline const auto& getIcon() const noexcept { return m_icon; }
  [[nodiscard]] inline const auto& getType() const noexcept { return m_type; }
  [[nodiscard]] inline bool isFavorite() const noexcept { return m_isFavorite; }
  [[nodiscard]] inline bool isDeleted() const noexcept { return m_isDeleted; }

  void updateIcon(const QIcon& newIcon) { m_icon = newIcon; }

 private:
  using VaultItemType = ikea400::dto::VaultItemType;

  QString m_name;
  QString m_id;
  QString m_customIconStr;
  QIcon m_icon;
  VaultItemType m_type;
  bool m_isFavorite;
  bool m_isDeleted;
};