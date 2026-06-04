#pragma once

#include <QLabel>

class HiddenLabel : public QLabel {
  Q_OBJECT
 public:
  explicit HiddenLabel(QWidget* parent = nullptr);
  ~HiddenLabel() noexcept;
  void setText(const QString& text);
  QString text() const;

  void setDisplayMode(bool hidden) noexcept;

  void updateDisplay() noexcept;

 private:
  QString m_text;
  QChar m_passwordCharacter;

  bool m_isHidden{true};

};