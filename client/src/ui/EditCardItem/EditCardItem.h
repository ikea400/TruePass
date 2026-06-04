#pragma once

#include <QAction>
#include <QWidget>

#include "../../model/CardItemDetailModel.h"
#include "ui_EditCardItem.h"

class EditCardItem : public QWidget {
  Q_OBJECT

 public:
  EditCardItem(QWidget* parent = nullptr);
  ~EditCardItem();

  void setModel(CardItemDetailModel* model);

 private:

 private:
  Ui::EditCardItemClass m_ui;

  CardItemDetailModel* m_model;
};
