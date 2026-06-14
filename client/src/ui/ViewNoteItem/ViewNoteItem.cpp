#include "ViewNoteItem.h"

#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QCheckBox>
#include <QIcon>
#include <QLayoutItem>

#include "../Components/CopyButton.h"
#include "../Components/PasswordRevealButton.h"
#include "../Components/HiddenLabel.h"

ViewNoteItem::ViewNoteItem(QWidget* parent) : QWidget(parent) {
  m_ui.setupUi(this);
}

ViewNoteItem::~ViewNoteItem() {}

void ViewNoteItem::setNoteDetails(const NoteItemDetailModel& details) {
  m_noteDetails = details;
  updateDisplay();
}

void ViewNoteItem::updateDisplay() noexcept {
  m_ui.noteContentEdit->setPlainText(m_noteDetails.getNoteContent());

  // Clear existing layout
  QLayoutItem* child;
  while ((child = m_ui.fieldsLayout->takeAt(0)) != nullptr) {
    if (QWidget* w = child->widget()) {
      w->deleteLater();
    }
    delete child;
  }

  for (const auto& field : m_noteDetails.getCustomFields()) {
    QGroupBox* groupBox = new QGroupBox(field.name, this);
    QHBoxLayout* layout = new QHBoxLayout(groupBox);
    layout->setContentsMargins(6, 6, 6, 6);
    layout->setSpacing(6);

    if (field.type == NoteItemDetailModel::FieldType::Text) {
      if (std::holds_alternative<QString>(field.value)) {
        QLabel* valLabel = new QLabel(std::get<QString>(field.value), groupBox);
        valLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);
        
        CopyButton* copyBtn = new CopyButton(groupBox);
        copyBtn->setIcon(QIcon(":/icons/icons/copy.svg"));
        copyBtn->setLabel(valLabel);
        copyBtn->setMaximumWidth(32);
        
        layout->addWidget(valLabel, 1);
        layout->addWidget(copyBtn, 0);
      }
    } else if (field.type == NoteItemDetailModel::FieldType::Password) {
      if (std::holds_alternative<QString>(field.value)) {
        HiddenLabel* valLabel = new HiddenLabel(groupBox);
        valLabel->setText(std::get<QString>(field.value));
        valLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);

        PasswordRevealButton* revealBtn = new PasswordRevealButton(groupBox);
        revealBtn->setPasswordField(valLabel);
        revealBtn->setMaximumWidth(32);

        CopyButton* copyBtn = new CopyButton(groupBox);
        copyBtn->setIcon(QIcon(":/icons/icons/copy.svg"));
        copyBtn->setLabel(valLabel);
        copyBtn->setMaximumWidth(32);

        layout->addWidget(valLabel, 1);
        layout->addWidget(revealBtn, 0);
        layout->addWidget(copyBtn, 0);
      }
    } else if (field.type == NoteItemDetailModel::FieldType::Boolean) {
      if (std::holds_alternative<bool>(field.value)) {
        QCheckBox* check = new QCheckBox(groupBox);
        check->setChecked(std::get<bool>(field.value));
        check->setEnabled(false);

        layout->addWidget(check, 1);
      }
    }

    m_ui.fieldsLayout->addWidget(groupBox);
  }
}
