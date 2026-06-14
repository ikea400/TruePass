#pragma once

#include <QWidget>

#include "../../model/IdentityItemDetailModel.h"
#include "ui_ViewIdentityItem.h"

class ViewIdentityItem : public QWidget {
  Q_OBJECT

 public:
  ViewIdentityItem(QWidget* parent = nullptr);
  ~ViewIdentityItem();

 public slots:
  void setIdentityDetails(const IdentityItemDetailModel& details);

  void updateDisplay() noexcept;

 private:
  Ui::ViewIdentityItemClass m_ui;

  IdentityItemDetailModel m_identityDetails;
};
