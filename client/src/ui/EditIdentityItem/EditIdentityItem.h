#pragma once

#include <QWidget>

#include "../../model/IdentityItemDetailModel.h"
#include "ui_EditIdentityItem.h"

class EditIdentityItem : public QWidget {
  Q_OBJECT

 public:
  EditIdentityItem(QWidget* parent = nullptr);
  ~EditIdentityItem();

  void setModel(IdentityItemDetailModel* model);

 private:
  Ui::EditIdentityItemClass m_ui;

  IdentityItemDetailModel* m_model;
};
