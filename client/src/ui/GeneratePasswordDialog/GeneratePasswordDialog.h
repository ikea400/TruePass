#pragma once

#include <QDialog>

#include "ui_GeneratePasswordDialog.h"

class GeneratePasswordDialog : public QDialog {
  Q_OBJECT

 public:
  GeneratePasswordDialog(QWidget *parent = nullptr);
  ~GeneratePasswordDialog();

 signals:
  void passwordGenerated(const QString &password);

 public slots:
  void onPasswordTypeToggled(bool checked);
  void onPassphraseTypeToggled(bool checked);
  void onPasswordSettingsChanged();
  void onConfirmButtonClicked();
  void onCancelButtonClicked();

 protected:
  virtual void showEvent(QShowEvent *event) override;

 private:
  void generatePassword();

 private:
  Ui::GeneratePasswordDialogClass ui;
};
