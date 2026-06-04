#pragma once
#include <QWidget>

#include "LottieAnimation.h"

class LoadingWidget : public QWidget {
  Q_OBJECT
 public:
  explicit LoadingWidget(QWidget* parent = nullptr);

  virtual ~LoadingWidget() noexcept;

  void start();
  void stop();
  void reset();

 protected:
  virtual void paintEvent(QPaintEvent* event) override;

 private:
  LottieAnimation m_animation;
};