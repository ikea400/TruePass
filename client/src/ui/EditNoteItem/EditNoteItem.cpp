#include "EditNoteItem.h"

#include <QHBoxLayout>
#include <QLineEdit>
#include <QComboBox>
#include <QCheckBox>
#include <QPushButton>
#include <QStackedWidget>
#include <QIcon>

EditNoteItem::EditNoteItem(QWidget *parent)
    : QWidget(parent)
{
  ui.setupUi(this);

  connect(ui.addFieldButton, &QPushButton::clicked, this, &EditNoteItem::onAddField);
  connect(ui.noteInput, &QPlainTextEdit::textChanged, this, &EditNoteItem::updateModel);
}

EditNoteItem::~EditNoteItem() {}

void EditNoteItem::setModel(NoteItemDetailModel* model) {
  m_model = model;
  populateUI();
}

void EditNoteItem::populateUI() {
  m_isPopulating = true;
  if (m_model) {
    ui.noteInput->setPlainText(m_model->getNoteContent());

    // Clear existing fields
    QLayoutItem* child;
    while ((child = ui.fieldsLayout->takeAt(0)) != nullptr) {
      if (QWidget* w = child->widget()) {
        w->deleteLater();
      }
      delete child;
    }

    for (const auto& field : m_model->getCustomFields()) {
      createFieldRowWidget(field);
    }
  } else {
    ui.noteInput->clear();
    QLayoutItem* child;
    while ((child = ui.fieldsLayout->takeAt(0)) != nullptr) {
      if (QWidget* w = child->widget()) {
        w->deleteLater();
      }
      delete child;
    }
  }
  m_isPopulating = false;
}

void EditNoteItem::updateModel() {
  if (m_isPopulating || !m_model) return;

  m_model->setNoteContent(ui.noteInput->toPlainText());

  QList<NoteItemDetailModel::CustomField> fields;
  for (int i = 0; i < ui.fieldsLayout->count(); ++i) {
    QLayoutItem* item = ui.fieldsLayout->itemAt(i);
    if (QWidget* rowWidget = item->widget()) {
      QLineEdit* nameInput = rowWidget->findChild<QLineEdit*>("nameInput");
      QComboBox* typeCombo = rowWidget->findChild<QComboBox*>("typeCombo");
      QStackedWidget* valueStack = rowWidget->findChild<QStackedWidget*>("valueStack");

      if (nameInput && typeCombo && valueStack) {
        NoteItemDetailModel::CustomField field;
        field.name = nameInput->text();

        int typeIndex = typeCombo->currentIndex();
        if (typeIndex == 1) {
          field.type = NoteItemDetailModel::FieldType::Password;
          QLineEdit* passInput = valueStack->findChild<QLineEdit*>("passInput");
          field.value = passInput ? passInput->text() : QString("");
        } else if (typeIndex == 2) {
          field.type = NoteItemDetailModel::FieldType::Boolean;
          QCheckBox* boolInput = valueStack->findChild<QCheckBox*>("boolInput");
          field.value = (boolInput && boolInput->isChecked());
        } else {
          field.type = NoteItemDetailModel::FieldType::Text;
          QLineEdit* textInput = valueStack->findChild<QLineEdit*>("textInput");
          field.value = textInput ? textInput->text() : QString("");
        }

        fields.append(field);
      }
    }
  }
  m_model->setCustomFields(fields);
}

void EditNoteItem::onAddField() {
  NoteItemDetailModel::CustomField field{"", NoteItemDetailModel::FieldType::Text, QString("")};
  createFieldRowWidget(field);
  updateModel();
}

void EditNoteItem::onRemoveField(QWidget* rowWidget) {
  if (rowWidget) {
    ui.fieldsLayout->removeWidget(rowWidget);
    rowWidget->deleteLater();
    updateModel();
  }
}

QWidget* EditNoteItem::createFieldRowWidget(const NoteItemDetailModel::CustomField& field) {
  QWidget* rowWidget = new QWidget(this);
  QHBoxLayout* layout = new QHBoxLayout(rowWidget);
  layout->setContentsMargins(0, 0, 0, 0);
  layout->setSpacing(6);

  QLineEdit* nameInput = new QLineEdit(rowWidget);
  nameInput->setObjectName("nameInput");
  nameInput->setPlaceholderText("Field Name");
  nameInput->setText(field.name);

  QComboBox* typeCombo = new QComboBox(rowWidget);
  typeCombo->setObjectName("typeCombo");
  typeCombo->addItems({"Text", "Password", "Boolean"});

  QStackedWidget* valueStack = new QStackedWidget(rowWidget);
  valueStack->setObjectName("valueStack");

  QLineEdit* textInput = new QLineEdit(valueStack);
  textInput->setObjectName("textInput");
  textInput->setPlaceholderText("Value");

  QLineEdit* passInput = new QLineEdit(valueStack);
  passInput->setObjectName("passInput");
  passInput->setEchoMode(QLineEdit::Password);
  passInput->setPlaceholderText("Password");

  QCheckBox* boolInput = new QCheckBox(valueStack);
  boolInput->setObjectName("boolInput");

  valueStack->addWidget(textInput);
  valueStack->addWidget(passInput);
  valueStack->addWidget(boolInput);

  if (field.type == NoteItemDetailModel::FieldType::Password) {
    typeCombo->setCurrentIndex(1);
    if (std::holds_alternative<QString>(field.value)) {
      passInput->setText(std::get<QString>(field.value));
    }
    valueStack->setCurrentIndex(1);
  } else if (field.type == NoteItemDetailModel::FieldType::Boolean) {
    typeCombo->setCurrentIndex(2);
    if (std::holds_alternative<bool>(field.value)) {
      boolInput->setChecked(std::get<bool>(field.value));
    }
    valueStack->setCurrentIndex(2);
  } else {
    typeCombo->setCurrentIndex(0);
    if (std::holds_alternative<QString>(field.value)) {
      textInput->setText(std::get<QString>(field.value));
    }
    valueStack->setCurrentIndex(0);
  }

  connect(typeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this,
          [this, valueStack](int index) {
            valueStack->setCurrentIndex(index);
            updateModel();
          });
          
  connect(nameInput, &QLineEdit::textChanged, this, &EditNoteItem::updateModel);
  connect(textInput, &QLineEdit::textChanged, this, &EditNoteItem::updateModel);
  connect(passInput, &QLineEdit::textChanged, this, &EditNoteItem::updateModel);
  connect(boolInput, &QCheckBox::checkStateChanged, this, &EditNoteItem::updateModel);

  QPushButton* deleteBtn = new QPushButton(rowWidget);
  deleteBtn->setIcon(QIcon(":/icons/icons/trash.svg"));
  deleteBtn->setToolTip("Delete Field");
  deleteBtn->setMaximumWidth(32);
  connect(deleteBtn, &QPushButton::clicked, this, [this, rowWidget]() {
    onRemoveField(rowWidget);
  });

  layout->addWidget(nameInput, 2);
  layout->addWidget(typeCombo, 1);
  layout->addWidget(valueStack, 2);
  layout->addWidget(deleteBtn, 0);

  ui.fieldsLayout->addWidget(rowWidget);
  return rowWidget;
}
