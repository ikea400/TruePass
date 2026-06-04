#include "LoadingButton.h"

#include <thorvg.h>

#include <QFile>
#include <QPainter>

LoadingButton::LoadingButton(QWidget* parent) noexcept
    : QPushButton(parent),
      m_loading(false),
      m_animation(this, ":/icons/icons/loading.json")
{
  qDebug("LoadingButton created");


  QObject::connect(this, &QPushButton::clicked, this,
                   &LoadingButton::onClicked);
}

LoadingButton::~LoadingButton() noexcept { tvg::Initializer::term(); }

void LoadingButton::setText(const QString& text) {
  QPushButton::setText(text);
  m_originalText = text;
}

void LoadingButton::startLoading() noexcept {
  if (m_loading) return;
  m_loading = true;

  QPushButton::setText("");

  m_animation.start();
}

void LoadingButton::stopLoading() noexcept {
  m_loading = false;
  QPushButton::setText(m_originalText);

  m_animation.stop();
}

void LoadingButton::paintEvent(QPaintEvent* event) {
  QPushButton::paintEvent(event);

  if (m_loading) {
    m_animation.onPaintEvent(QSize(width(), height() * 3.f));
  }
}

void LoadingButton::onClicked() noexcept {
  if (m_loading) return;

  startLoading();
  emit loadingStarted();
}
