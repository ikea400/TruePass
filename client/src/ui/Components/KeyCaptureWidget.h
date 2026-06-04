#pragma once

#include <QLineEdit>

class KeyCaptureWidget : public QLineEdit {
  Q_OBJECT
 public:
  KeyCaptureWidget(QWidget* parent = nullptr);
  ~KeyCaptureWidget() noexcept = default;

  QChar currentCharacter() const noexcept { return m_currentChar; }

  void setCurrentCharacter(QChar c) noexcept;

 signals:
  void characterChanged(QChar c);

protected:
  void keyPressEvent(QKeyEvent* event) override;

 private:
  QChar m_noKeyChar{u' '};
  QChar m_currentChar{u'-'};
};