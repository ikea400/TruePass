#pragma once

#include <QDialog>

#include "../../model/VaultItemModel.h"
#include "ui_EditItemDialog.h"

class EditItemDialog : public QDialog {
  Q_OBJECT

 public:
  EditItemDialog(QWidget* parent = nullptr);
  ~EditItemDialog();

  void setModel(const VaultItemModel& model);

 signals:
  void itemUpdated(const VaultItemModel& updatedItem);

 public slots:
  void onConfirm();
  void onCancel();
  void onItemEdited();
  void onEditItemError(const QString& error);

 private:
  QString validateName(const QString& name);

  void updateError(const QString& error);

 private:
  Ui::EditItemDialogClass ui;

  VaultItemModel m_model;
};
