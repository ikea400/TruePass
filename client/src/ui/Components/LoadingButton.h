#pragma once
#include <thorvg.h>

#include <QIcon>
#include <QPushButton>
#include <QString>
#include <QSvgRenderer>
#include <QTimer>
#include <QWidget>
#include <memory>

#include "LottieAnimation.h"

class LoadingButton : public QPushButton {
  Q_OBJECT

 public:
  LoadingButton(QWidget* parent = nullptr) noexcept;

  virtual ~LoadingButton() noexcept;

  void setText(const QString& text);

  void startLoading() noexcept;
  void stopLoading() noexcept;

 signals:
  void loadingStarted();

 public slots:
  void onClicked() noexcept;

 protected:
  virtual void paintEvent(QPaintEvent* event) override;

 private:
  QString m_originalText;
  bool m_loading;

  LottieAnimation m_animation;
};