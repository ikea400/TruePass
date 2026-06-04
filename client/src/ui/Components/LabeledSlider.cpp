#include "LabeledSlider.h"

LabeledSlider::LabeledSlider(QWidget* parent) : QSlider(parent) {
  updateLabel();

  connect(this, &QSlider::valueChanged, this, &LabeledSlider::onValueChanged);
}

void LabeledSlider::setLabel(QLabel* label) noexcept {
  m_label = label;
  updateLabel();
}

void LabeledSlider::setFormat(const QString& format) noexcept {
  m_format = format;
  updateLabel();
}

void LabeledSlider::updateLabel() noexcept {
  if (m_label) {
    m_label->setText(m_format.isEmpty() ? QString::number(value())
                                        : m_format.arg(value()));
  }
}

void LabeledSlider::onValueChanged(int value) noexcept { updateLabel(); }
