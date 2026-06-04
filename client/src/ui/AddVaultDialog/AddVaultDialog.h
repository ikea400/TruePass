#pragma once
#include <QDialog>

#include "ui_AddVaultDialog.h"

class AddVaultDialog : public QDialog {
  Q_OBJECT

 public:
  AddVaultDialog(QWidget* parent = nullptr);
  ~AddVaultDialog();

signals:
  void createVault(const QString& name, const QString& description);

 public slots:
  void onNameEditingFinished();
  void onDescriptionEditingFinished();
  void onConfirmButtonClicked();
  void onVaultAdded();
  void onAddVaultError(const QString& error);

 private:
  void inputValidation();

  void updateError(const QString& error);

 private:
  Ui::AddVaultDialogClass ui;
};
