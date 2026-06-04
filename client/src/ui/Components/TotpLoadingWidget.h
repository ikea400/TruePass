#pragma once
#include <QWidget>

#include "LottieAnimation.h"

class TotpLoadingWidget : public QWidget {
  Q_OBJECT

 public:
  TotpLoadingWidget(QWidget* parent = nullptr);
  ~TotpLoadingWidget();

 protected:
  virtual void paintEvent(QPaintEvent* event) override;

  static float customFrameStep(float currentFrame, float totalFrames);

 private:
  LottieAnimation m_animation;
};
