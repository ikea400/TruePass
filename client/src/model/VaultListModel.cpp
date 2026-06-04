#include "VaultListModel.h"

#include <qabstractitemmodel.h>
#include <qnamespace.h>
#include <qobject.h>
#include <qvariant.h>
#include <utils/uuid.h>

#include <algorithm>
#include <iterator>
#include <unordered_map>
#include <utility>

#include "../core/Vault.h"
#include "VaultModel.h"

VaultListModel::VaultListModel(QObject* parent) : QAbstractListModel(parent) {}

int VaultListModel::rowCount(const QModelIndex& parent) const {
  if (parent.isValid()) return 0;
  if (m_vaults.size() > static_cast<size_t>(std::numeric_limits<int>::max())) {
    qWarning() << "Too many vaults in model";
    return 0;
  }
  return static_cast<int>(m_vaults.size());
}

QVariant VaultListModel::data(const QModelIndex& index, int role) const {
  if (index.row() < 0 || index.row() >= rowCount()) return QVariant();

  const VaultModel& vault = m_vaults[index.row()];
  switch (role) {
    case Qt::DisplayRole:
      return vault.getName();
    case Qt::ToolTipRole:
      return vault.getDescription();
    case Qt::UserRole:
      return vault.getId();
    default:
      return QVariant();
  }
}

void VaultListModel::setVaults(
    const std::unordered_map<ikea400::uuid, Vault>& vaults) {
  beginResetModel();
  m_vaults.clear();

  for (const auto& [id, vault] : vaults) {
    m_vaults.emplace_back(vault.getName(), vault.getId().toString(),
                          vault.getDescription());
  }

  std::sort(m_vaults.begin(), m_vaults.end(),
            [](const VaultModel& a, const VaultModel& b) {
              return a.getName() < b.getName();
            });

  endResetModel();
}

void VaultListModel::addVault(const Vault& vault) {
  VaultModel newVault(vault.getName(), vault.getId().toString(),
                      vault.getDescription());

  auto it = std::lower_bound(m_vaults.begin(), m_vaults.end(), newVault,
                             [](const VaultModel& a, const VaultModel& b) {
                               return a.getName() < b.getName();
                             });

  int row = std::distance(m_vaults.begin(), it);

  beginInsertRows(QModelIndex(), row, row);
  m_vaults.insert(m_vaults.begin() + row, std::move(newVault));
  endInsertRows();
}
