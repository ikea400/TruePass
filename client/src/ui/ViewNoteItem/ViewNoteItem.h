#pragma once

#include <QWidget>
#include <QList>
#include "../../model/NoteItemDetailModel.h"
#include "ui_ViewNoteItem.h"

class ViewNoteItem : public QWidget {
  Q_OBJECT

 public:
  explicit ViewNoteItem(QWidget* parent = nullptr);
  ~ViewNoteItem();

  void setNoteDetails(const NoteItemDetailModel& details);

  void updateDisplay() noexcept;

 private:
  Ui::ViewNoteItemClass m_ui;
  NoteItemDetailModel m_noteDetails;
};
