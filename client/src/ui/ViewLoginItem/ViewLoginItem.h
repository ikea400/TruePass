#pragma once

#include <QWidget>

#include "../../model/LoginItemDetailModel.h"
#include "ui_ViewLoginItem.h"

class ViewLoginItem : public QWidget {
  Q_OBJECT

 public:
  ViewLoginItem(QWidget* parent = nullptr);
  ~ViewLoginItem();

 public slots:
  void setLoginDetails(const LoginItemDetailModel& details);

  void updateDisplay() noexcept;

 private:
  Ui::ViewLoginItemClass m_ui;

  LoginItemDetailModel m_loginDetails;
};
