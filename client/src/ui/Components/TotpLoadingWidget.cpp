#include "TotpLoadingWidget.h"

#include <chrono>

#include "../../core/utils.h"

TotpLoadingWidget::TotpLoadingWidget(QWidget* parent)
    : QWidget(parent),
      m_animation(this, ":/icons/icons/totp.json", customFrameStep) {
  qDebug() << "TotpLoadingWidget created with size " << size();

  ikea400::utils::startSyncedTimer([this]() { m_animation.start(1000.f); });
}

TotpLoadingWidget::~TotpLoadingWidget() {}

void TotpLoadingWidget::paintEvent(QPaintEvent* event) {
  QWidget::paintEvent(event);

  m_animation.onPaintEvent(QSize(24, 24));
}

float TotpLoadingWidget::customFrameStep(float /*currentFrame*/,
                                         float totalFrames) {
  if (totalFrames <= 0.0f) return 0.0f;

  constexpr const int TOTP_PERIOD_S = 30;

  auto now = std::chrono::system_clock::now();
  auto s =
      std::chrono::duration_cast<std::chrono::seconds>(now.time_since_epoch())
          .count();

  int sInWindow = s % TOTP_PERIOD_S;

  float progress = static_cast<float>(sInWindow) / TOTP_PERIOD_S;

  return progress * totalFrames;
}
