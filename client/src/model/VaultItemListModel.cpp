#include "VaultItemListModel.h"

#include <qabstractitemmodel.h>
#include <qicon.h>
#include <qnamespace.h>
#include <qobject.h>
#include <qvariant.h>
#include <utils/uuid.h>

#include <algorithm>
#include <iterator>
#include <unordered_map>
#include <utility>

#include "../core/VaultItem.h"
#include "VaultItemSummaryModel.h"

VaultItemListModel::VaultItemListModel(QObject* parent)
    : QAbstractListModel(parent), m_iconManager(new IconProvider()) {
  connect(m_iconManager, &IconProvider::iconUpdated, this,
          &VaultItemListModel::onIconUpdated);
}

int VaultItemListModel::rowCount(const QModelIndex& parent) const {
  if (parent.isValid()) return 0;
  if (!m_errorMessage.isEmpty()) return 1;
  if (m_items.size() > static_cast<size_t>(std::numeric_limits<int>::max())) {
    qWarning() << "Too many items in model";
    return 0;
  }
  return static_cast<int>(m_items.size());
}

QVariant VaultItemListModel::data(const QModelIndex& index, int role) const {
  if (index.row() < 0 || index.row() >= rowCount()) return QVariant();

  if (!m_errorMessage.isEmpty()) {
    if (role == Qt::DisplayRole) return m_errorMessage;
    if (role == IsErrorRole) return true;
    return QVariant();
  }

  const VaultItemSummaryModel& item = m_items[index.row()];
  switch (role) {
    case Qt::DisplayRole:
      return item.getName();
    case Qt::DecorationRole:
      return item.getIcon();
    case IdRole:
      return item.getId();
    case NameRole:
      return item.getName();
    case TypeRole:
      return QVariant::fromValue(item.getType());
    case IsFavoriteRole:
      return item.isFavorite();
    case IsDeletedRole:
      return item.isDeleted();
  }

  return QVariant();
}

void VaultItemListModel::setItems(
    const std::unordered_map<ikea400::uuid, VaultItem>& items) {
  beginResetModel();
  m_errorMessage.clear();
  m_items.clear();
  for (const auto& [id, item] : items) {
    QIcon icon = m_iconManager->getItemIcon(item);

    m_items.emplace_back(item.getName(), item.getId().toString(),
                         item.getCustomIcon(), icon, item.getType(),
                         item.isFavorite(), item.isDeleted());
  }

  std::sort(m_items.begin(), m_items.end(), compareItems);

  endResetModel();
}

void VaultItemListModel::addItem(const VaultItem& item) {
  qDebug() << "Adding item:" << QString::fromStdString(item.getName())
           << "with ID:" << QString::fromStdString(item.getId().toString());

  QIcon icon = m_iconManager->getItemIcon(item);

  VaultItemSummaryModel newVault(item.getName(), item.getId().toString(),
                                 item.getCustomIcon(), icon, item.getType(),
                                 item.isFavorite(), item.isDeleted());

  auto it =
      std::lower_bound(m_items.begin(), m_items.end(), newVault, compareItems);

  int row = std::distance(m_items.begin(), it);

  beginInsertRows(QModelIndex(), row, row);
  m_items.insert(m_items.begin() + row, newVault);
  endInsertRows();
}

void VaultItemListModel::updateItem(const VaultItem& item) {
  QString itemId = QString::fromStdString(item.getId().toString());
  auto it = std::find_if(m_items.begin(), m_items.end(),
                         [&itemId](const VaultItemSummaryModel& summary) {
                           return summary.getId() == itemId;
                         });
  if (it == m_items.end()) {
    qWarning()
        << "Attempted to update item that doesn't exist in model with ID:"
        << itemId;
    return;
  }

  int row = std::distance(m_items.begin(), it);

  QIcon icon = m_iconManager->getItemIcon(item);

  *it = VaultItemSummaryModel(item.getName(), item.getId().toString(),
                              item.getCustomIcon(), icon, item.getType(),
                              item.isFavorite(), item.isDeleted());

  emit dataChanged(index(row), index(row));
}

void VaultItemListModel::clearItems() {
  beginResetModel();
  m_errorMessage.clear();
  m_items.clear();
  endResetModel();
}

void VaultItemListModel::enterErrorMode(const QString& error) {
  beginResetModel();
  m_errorMessage = error;
  m_items.clear();
  endResetModel();
}

void VaultItemListModel::onIconUpdated(const QString& customIcon) {
  qDebug() << "Icon updated for:" << customIcon;
  for (int i = 0; i < m_items.size(); ++i) {
    if (m_items[i].getCustomIconStr() == customIcon) {
      m_items[i].updateIcon(m_iconManager->getItemIcon(m_items[i]));

      QModelIndex idx = index(i);
      emit dataChanged(idx, idx, {Qt::DecorationRole});
    }
  }
}

QIcon VaultItemListModel::getIconForType(
    ikea400::dto::VaultItemType type) const {
  QIcon icon;
  switch (type) {
    case ikea400::dto::VaultItemType::Login:
      icon.addFile(QStringLiteral(":/icons/icons/login.svg"), QSize(),
                   QIcon::Normal, QIcon::Off);
      break;
    default:
      break;
  }
  return icon;
}
