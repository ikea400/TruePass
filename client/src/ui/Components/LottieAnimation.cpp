#include "LottieAnimation.h"

#include <QFile>
#include <QPainter>
#include <algorithm>
#include <cstdint>
#include <functional>
#include <memory>
#include <utility>

#define SIZE_TEST 1

LottieAnimation::LottieAnimation(
    QWidget* parent, const QString& path,
    std::function<float(float, float)> customFrameStep)
    : m_animationWidget(parent),
      m_canvas(nullptr),
      m_animation(tvg::Animation::gen()),
      m_frameTimer(new QTimer(parent)) {
  tvg::Initializer::init(0);
  setupAnimation(path);

  if (customFrameStep) {
    m_currentFrame = customFrameStep(m_currentFrame, m_totalFrames);
    onFrameUpdate();
  }

  QObject::connect(m_frameTimer, &QTimer::timeout,
                   [this, customFrameStep = std::move(customFrameStep)]() {
                     if (customFrameStep) {
                       m_currentFrame =
                           customFrameStep(m_currentFrame, m_totalFrames);
                     } else {
                       m_currentFrame++;
                     }

                     onFrameUpdate();
                   });
}

LottieAnimation::~LottieAnimation() { tvg::Initializer::term(); }

float LottieAnimation::duration() const {
  return m_animation ? m_animation->duration() : 0.f;
}

void LottieAnimation::start(float frameTime) {
  if (m_totalFrames > 0) {
    // Use duration from API to set timer interval
    m_frameTimer->start(
        frameTime > 0.f
            ? frameTime
            : static_cast<int>((m_animation->duration() / m_totalFrames) *
                               1000));
  }

  m_animationWidget->update();
}

void LottieAnimation::pause() {
  m_frameTimer->stop();
  m_animationWidget->update();
}

void LottieAnimation::stop() {
  pause();
  reset();
}

void LottieAnimation::reset() { m_currentFrame = 0; }

void LottieAnimation::setVisible(bool visible) { m_isVisible = visible; }

void LottieAnimation::onPaintEvent(const QSize& size) {
#if SIZE_TEST
  if (!m_isVisible || !m_animation) return;

  if (!m_canvas || size != m_wantedSize) {
    m_wantedSize = size;

    m_actualSize = calculateProportionalSize(size);

    m_buffer.resize(static_cast<size_t>(m_actualSize.width()) *
                    static_cast<size_t>(m_actualSize.height()) *
                    sizeof(uint32_t));
    std::fill(m_buffer.begin(), m_buffer.end(), 0);

    auto picture = m_animation->picture();
    picture->size(m_actualSize.width(), m_actualSize.height());

    bool first = !m_canvas;
    if (first) m_canvas = std::unique_ptr<tvg::SwCanvas>(tvg::SwCanvas::gen());
    m_canvas->target(m_buffer.data(), m_actualSize.width(),
                     m_actualSize.height(), m_actualSize.width(),
                     tvg::ColorSpace::ARGB8888);
    if (first) m_canvas->add(picture);
  }

  if (!m_canvas) return;

  m_animation->frame(m_currentFrame);
  m_canvas->update();
  m_canvas->draw();
  m_canvas->sync();
  QPainter painter(m_animationWidget);
  QImage img(reinterpret_cast<const uchar*>(m_buffer.data()),
             m_actualSize.width(), m_actualSize.height(),
             QImage::Format_ARGB32);

  int x = (m_animationWidget->width() - img.width()) / 2;
  int y = (m_animationWidget->height() - img.height()) / 2;

  painter.drawImage(x, y, img);
#else

  if (!m_isVisible || !m_canvas || !m_animation) return;

  m_animation->frame(m_currentFrame);
  m_canvas->update();
  m_canvas->draw();
  m_canvas->sync();
  QPainter painter(m_animationWidget);
  QImage img(reinterpret_cast<const uchar*>(m_buffer.data()), m_width, m_height,
             QImage::Format_ARGB32);

  QImage proportional =
      img.scaled(size, Qt::KeepAspectRatio, Qt::SmoothTransformation);

  int x = (m_animationWidget->width() - proportional.width()) / 2;
  int y = (m_animationWidget->height() - proportional.height()) / 2;

  painter.drawImage(x, y, proportional);
#endif
}

void LottieAnimation::setupAnimation(const QString& path) {
  auto picture = m_animation->picture();

  QFile file(path);
  if (!file.open(QIODevice::ReadOnly)) {
    qWarning() << "Failed to open lottie file:" << path;
    return;
  }
  QByteArray data = file.readAll();

  // Load data into picture (true = copy data)
  if (picture->load(data.data(), data.size(), "lot", nullptr, true) !=
      tvg::Result::Success) {
    qDebug() << "Failed to load lottie file: " << path;
    return;
  }

#if SIZE_TEST
  m_totalFrames = m_animation->totalFrame();
  picture->size(&m_originalWidth, &m_originalHeight);
#else

  picture->size(2048.f, 2048.f);
  picture->size(&m_width, &m_height);
  m_totalFrames = m_animation->totalFrame();

  m_buffer.resize(m_width * m_height * sizeof(uint32_t));
  std::fill(m_buffer.begin(), m_buffer.end(), 0);  // Clear buffer with zeros

  m_canvas = std::unique_ptr<tvg::SwCanvas>(tvg::SwCanvas::gen());

  m_canvas->target(m_buffer.data(), m_width, m_height, m_width,
                   tvg::ColorSpace::ARGB8888);

  m_canvas->add(picture);
#endif
}

void LottieAnimation::onFrameUpdate() {
  if (m_currentFrame >= m_totalFrames) m_currentFrame = 0;
  m_animationWidget->update();  // Triggers paintEvent
}

QSize LottieAnimation::calculateProportionalSize(
    const QSize& requestedSize) const {
  float widthRatio = requestedSize.width() / m_originalWidth;
  float heightRatio = requestedSize.height() / m_originalHeight;
  float scaleFactor = std::min(widthRatio, heightRatio);

  return QSize(m_originalWidth * scaleFactor, m_originalHeight * scaleFactor);
}
