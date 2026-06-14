#pragma once

#include <QString>
#include <QList>
#include <variant>

class NoteItemDetailModel {
 public:
  enum class FieldType {
    Text,
    Password,
    Boolean
  };

  using FieldValue = std::variant<QString, bool>;

  struct CustomField {
    QString name;
    FieldType type;
    FieldValue value;
  };

  const QString& getNoteContent() const noexcept { return m_noteContent; }
  const QList<CustomField>& getCustomFields() const noexcept { return m_customFields; }

  void setNoteContent(const QString& noteContent) { m_noteContent = noteContent; }
  void setCustomFields(const QList<CustomField>& customFields) { m_customFields = customFields; }

 private:
  QString m_noteContent;
  QList<CustomField> m_customFields;
};
