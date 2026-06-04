#pragma once

#include <QWidget>

#include "../../model/LoginItemDetailModel.h"
#include "ui_EditLoginItem.h"

class GeneratePasswordDialog;

class EditLoginItem : public QWidget {
  Q_OBJECT

 public:
  EditLoginItem(QWidget* parent = nullptr);
  ~EditLoginItem();

  void setModel(LoginItemDetailModel* model);

 public slots:
  void onUsernameChanged(const QString& username);
  void onEmailChanged(const QString& email);
  void onPasswordChanged(const QString& password);
  void onTotpSecretChanged(const QString& totpSecret);
  void onWebsiteChanged(const QString& website);
  void onGeneratePasswordClicked();
  void onClose();

 private:
  Ui::EditLoginItemClass ui;

  LoginItemDetailModel* m_model{nullptr};
  GeneratePasswordDialog* m_generatePasswordDialog{nullptr};
};
