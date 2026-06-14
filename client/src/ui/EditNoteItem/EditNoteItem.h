#pragma once

#include <QWidget>
#include <QList>
#include <QString>
#include <QStackedWidget>
#include "ui_EditNoteItem.h"
#include "../../model/NoteItemDetailModel.h"

class EditNoteItem : public QWidget {
  Q_OBJECT

 public:
  explicit EditNoteItem(QWidget *parent = nullptr);
  ~EditNoteItem();

  void setModel(NoteItemDetailModel* model);

 private slots:
  void onAddField();
  void onRemoveField(QWidget* rowWidget);
  void updateModel();

 private:
  void populateUI();
  QWidget* createFieldRowWidget(const NoteItemDetailModel::CustomField& field);

  Ui::EditNoteItemClass ui;
  NoteItemDetailModel* m_model = nullptr;
  bool m_isPopulating = false;
};
