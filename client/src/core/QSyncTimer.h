#pragma once
#include <QDateTime>
#include <QTimer>

class QSyncTimer : public QObject {
  Q_OBJECT
  bool m_running = false;

 public:
  explicit QSyncTimer(QObject* parent = nullptr) : QObject(parent) {}

  void start() {
    if (m_running) return;
    m_running = true;
    scheduleNext();
  }

  void stop() { m_running = false; }

 signals:
  void syncTimeout();

 private:
  void scheduleNext() {
    if (!m_running) return;

    const qint64 now = QDateTime::currentMSecsSinceEpoch();
    const qint64 next = ((now / 1000) + 1) * 1000;

    QTimer::singleShot(static_cast<int>(next - now), this, [this] {
      if (!m_running) return;
      emit syncTimeout();
      scheduleNext();
    });
  }
};