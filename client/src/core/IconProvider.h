#pragma once
#include <QIcon>
#include <QObject>
#include <QString>

#include "../model/VaultItemSummaryModel.h"
#include "VaultItem.h"

class IconProvider : public QObject {
  Q_OBJECT
 public:
  explicit IconProvider(QObject* parent = nullptr);

  QIcon getItemIcon(ikea400::dto::VaultItemType type, const QString& customIcon) const;
  QIcon getItemIcon(const VaultItemSummaryModel& itemSummary) const;
  QIcon getItemIcon(const VaultItem& item) const;

  QIcon getCardItemIcon(const QString& brand) const;

 signals:
  void iconUpdated(const QString& customIcon);

 private:
  QIcon m_defaultLoginIcon;
  QIcon m_defaultCardIcon;
};