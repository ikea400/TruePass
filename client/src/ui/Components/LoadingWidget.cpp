#include "LoadingWidget.h"

LoadingWidget::LoadingWidget(QWidget* parent)
    : QWidget(parent), m_animation(this, ":/icons/icons/circle-spinner.json") {
  start();
}

LoadingWidget::~LoadingWidget() noexcept {}

void LoadingWidget::start() { m_animation.start(); }

void LoadingWidget::stop() { m_animation.stop(); }

void LoadingWidget::reset() { m_animation.reset(); }

void LoadingWidget::paintEvent(QPaintEvent* event) {
  QWidget::paintEvent(event);
  m_animation.onPaintEvent(size());
}
