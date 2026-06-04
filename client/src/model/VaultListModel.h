#pragma once
#include <utils/uuid.h>

#include <QAbstractListModel>
#include <unordered_map>
#include <vector>

#include "../core/Vault.h"
#include "VaultModel.h"

class VaultListModel : public QAbstractListModel {
  Q_OBJECT
 public:
  explicit VaultListModel(QObject* parent = nullptr);
  int rowCount(const QModelIndex& parent = QModelIndex()) const override;
  QVariant data(const QModelIndex& index,
                int role = Qt::DisplayRole) const override;

  void setVaults(const std::unordered_map<ikea400::uuid, Vault>& vaults);

  void addVault(const Vault& vault);

 private:
  std::vector<VaultModel> m_vaults;
};