#pragma once
#include <thorvg.h>

#include <QSize>
#include <QTimer>
#include <QWidget>
#include <memory>
#include <vector>
#include <functional>

class LottieAnimation {
 public:
  LottieAnimation(QWidget* parent, const QString& path, std::function<float(float, float)> customFrameStep  = nullptr);
  ~LottieAnimation();

  float duration() const;

  void start(float frameTime = 0.f);
  void pause();
  void stop();
  void reset();

  void setVisible(bool visible);

  void onPaintEvent(const QSize& size);

 private:
  void setupAnimation(const QString& path);

  void onFrameUpdate();

  QSize calculateProportionalSize(const QSize& requestedSize) const;

 private:
  QWidget* m_animationWidget;

  std::unique_ptr<tvg::SwCanvas> m_canvas;
  std::unique_ptr<tvg::Animation> m_animation;

  // Buffer for ThorVG to draw into
  std::vector<uint32_t> m_buffer;
  QTimer* m_frameTimer;
  QSize m_actualSize;
  QSize m_wantedSize;
  float m_currentFrame = 0.0f;
  float m_totalFrames = 0.0f;
  float m_originalWidth = 0.0f;
  float m_originalHeight = 0.0f;

  bool m_isVisible = true;
};