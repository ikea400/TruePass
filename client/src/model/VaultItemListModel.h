#pragma once
#include <qabstractitemmodel.h>
#include <qnamespace.h>
#include <qobject.h>
#include <qvariant.h>
#include <utils/uuid.h>

#include <unordered_map>
#include <vector>

#include "../core/IconProvider.h"
#include "../core/VaultItem.h"
#include "VaultItemSummaryModel.h"

enum VaultItemListRole {
  IdRole = Qt::UserRole,
  NameRole,
  TypeRole,
  IsFavoriteRole,
  IsDeletedRole,
  IsErrorRole
};

class VaultItemListModel : public QAbstractListModel {
  Q_OBJECT
 public:
  explicit VaultItemListModel(QObject* parent = nullptr);

  int rowCount(const QModelIndex& parent = QModelIndex()) const override;
  QVariant data(const QModelIndex& index,
                int role = Qt::DisplayRole) const override;

  void setItems(const std::unordered_map<ikea400::uuid, VaultItem>& items);

  void addItem(const VaultItem& item);
  void updateItem(const VaultItem& item);

  void clearItems();

  void enterErrorMode(const QString& error);

 public slots:
  void onIconUpdated(const QString& customIcon);

 private:
  QIcon getIconForType(ikea400::dto::VaultItemType type) const;

  static bool compareItems(const VaultItemSummaryModel& a,
                           const VaultItemSummaryModel& b) {
    return a.getName().compare(b.getName(), Qt::CaseInsensitive) < 0;
  }

 private:
  std::vector<VaultItemSummaryModel> m_items;
  QString m_errorMessage;
  IconProvider* m_iconManager;
};