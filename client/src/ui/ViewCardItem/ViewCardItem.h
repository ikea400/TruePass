#pragma once

#include <QWidget>

#include "../../model/CardItemDetailModel.h"
#include "ui_ViewCardItem.h"

class ViewCardItem : public QWidget {
  Q_OBJECT

 public:
  ViewCardItem(QWidget* parent = nullptr);
  ~ViewCardItem();

  void setCardDetails(const CardItemDetailModel& details);

 private:
  Ui::ViewCardItemClass ui;

  CardItemDetailModel m_cardDetails;
};
