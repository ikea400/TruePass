#pragma once
#include <QLabel>
#include <QSlider>

class LabeledSlider : public QSlider {
  Q_OBJECT
 public:
  LabeledSlider(QWidget* parent = nullptr);
  ~LabeledSlider() noexcept = default;

  void setLabel(QLabel* label) noexcept;
  void setFormat(const QString& format) noexcept;

  void updateLabel() noexcept;

 public slots:
  void onValueChanged(int value) noexcept;

 private:
  QString m_format{};
  QLabel* m_label{nullptr};
};