#include "VaultItemListProxy.h"

#include "../dto/VaultItemMetadataDto.h"
#include "VaultItemListModel.h"

VaultItemListProxy::VaultItemListProxy(QObject* parent)
    : QSortFilterProxyModel(parent) {}

void VaultItemListProxy::setFilterType(VaultItemFilterType filterType) {
  if (m_filterType != filterType) {
    m_filterType = filterType;
    invalidateFilter();
  }
}

void VaultItemListProxy::setSearchQuery(const QString& query) {
  if (m_searchQuery != query) {
    m_searchQuery = query;
    invalidateFilter();
  }
}

bool VaultItemListProxy::filterAcceptsRow(
    int source_row, const QModelIndex& source_parent) const {
  QModelIndex index = sourceModel()->index(source_row, 0, source_parent);
  if (!index.isValid()) {
    return false;
  }

  if (sourceModel()->data(index, IsErrorRole).toBool()) {
    return true;
  }

  return filterAcceptsItemType(index) && filterAcceptsSearchQuery(index);
}

bool VaultItemListProxy::filterAcceptsItemType(const QModelIndex& index) const {
  QVariant isDeletedData = sourceModel()->data(index, IsDeletedRole);
  bool isDeleted = isDeletedData.isValid() && isDeletedData.toBool();
  if (m_filterType == VaultItemFilterType::Deleted) return isDeleted;
  if (isDeleted) return false;

  switch (m_filterType) {
    case VaultItemFilterType::All:
      return true;
    case VaultItemFilterType::Favorites: {
      QVariant isFavoriteData = sourceModel()->data(index, IsFavoriteRole);
      return isFavoriteData.isValid() && isFavoriteData.toBool();
    }
    case VaultItemFilterType::Login: {
      QVariant typeData = sourceModel()->data(index, TypeRole);
      return typeData.isValid() &&
             typeData.value<VaultItemType>() == VaultItemType::Login;
    }
    case VaultItemFilterType::Card: {
      QVariant typeData = sourceModel()->data(index, TypeRole);
      return typeData.isValid() &&
             typeData.value<VaultItemType>() == VaultItemType::Card;
    }
    case VaultItemFilterType::Identity: {
      QVariant typeData = sourceModel()->data(index, TypeRole);
      return typeData.isValid() &&
             typeData.value<VaultItemType>() == VaultItemType::Identity;
    }
    case VaultItemFilterType::Note: {
      QVariant typeData = sourceModel()->data(index, TypeRole);
      return typeData.isValid() &&
             typeData.value<VaultItemType>() == VaultItemType::Note;
    }
  }

  return false;
}

bool VaultItemListProxy::filterAcceptsSearchQuery(
    const QModelIndex& index) const {
  if (m_searchQuery.isEmpty()) return true;

  QVariant nameData = sourceModel()->data(index, NameRole);
  return nameData.isValid() &&
         nameData.toString().contains(m_searchQuery, Qt::CaseInsensitive);
}
