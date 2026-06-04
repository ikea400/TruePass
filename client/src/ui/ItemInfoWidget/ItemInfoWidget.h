#pragma once
#include <QWidget>
#include <optional>

#include  <utils/uuid.h>

#include "../../model/VaultItemModel.h"
#include "../EditItemDialog/EditItemDialog.h"
#include "ui_ItemInfoWidget.h"

class ItemInfoWidget : public QWidget {
  Q_OBJECT

 public:
  ItemInfoWidget(QWidget* parent = nullptr);
  ~ItemInfoWidget();

signals:
  void editItem(const VaultItemModel& item, const ikea400::uuid& vaultId);

 public slots:
  void openEmptyPage();
  void openLoadingPage();
  void openErrorPage(const QString& error);
  void openItemInfoPage(const ikea400::uuid& vaultId, const VaultItemModel& item);
  void onEditClicked();
  void onItemUpdated(const VaultItemModel& updatedItem);
  void onItemEdited(const ikea400::uuid& vaultId, const VaultItemModel& item);
  void onEditItemError(const QString& error);

 private:
  void resetLabel();
  void enableButtons(bool enabled);
  void startLoadingAnimation();
  void stopLoadingAnimation();
  void updateItemDisplay();

 private:
  Ui::ItemInfoWidgetClass m_ui;

  std::optional<VaultItemModel> m_currentModel;
  ikea400::uuid m_currentVaultId;

  EditItemDialog* m_editDialog{nullptr};
};
