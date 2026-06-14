#pragma once
#include <QSortFilterProxyModel>

#include "../dto/VaultItemMetadataDto.h"

enum class VaultItemFilterType { All, Favorites, Deleted, Login, Card, Identity, Note };

class VaultItemListProxy : public QSortFilterProxyModel {
  Q_OBJECT
 public:
  using VaultItemType = ikea400::dto::VaultItemType;

  explicit VaultItemListProxy(QObject* parent = nullptr);

  void setFilterType(VaultItemFilterType filterType);
  void setSearchQuery(const QString& query);

 protected:
  bool filterAcceptsRow(int source_row,
                        const QModelIndex& source_parent) const override;

  bool filterAcceptsItemType(const QModelIndex& index) const;
  bool filterAcceptsSearchQuery(const QModelIndex& index) const;

 private:
  VaultItemFilterType m_filterType{VaultItemFilterType::All};
  QString m_searchQuery;
};