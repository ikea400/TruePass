#pragma once

#include <QDialog>

#include "../../model/VaultItemModel.h"
#include "ui_AddItemDialog.h"

class AddItemDialog : public QDialog {
  Q_OBJECT

 public:
  AddItemDialog(QWidget* parent = nullptr);
  ~AddItemDialog();

 signals:
  void addItem(const VaultItemModel& item);
  void closing();

 public slots:
  void onCurrentIndexChanged(int index);
  void onConfirmClicked();
  void onNameChanged(const QString& name);
  void onNameEdited();
  void onNoteChanged();
  void onAddItemError(const QString& error);
  void onItemAdded();

  void validateName();

  void updateNameError(const QString& error);
  void updateGlobalError(const QString& error);

 protected:
  virtual void closeEvent(QCloseEvent* e) override;

 private:
  Ui::AddItemDialogClass ui;

  VaultItemModel m_model;
};
