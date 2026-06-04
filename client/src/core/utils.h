#pragma once
#include <qdatetime.h>
#include <qtimer.h>
#include <qtypes.h>

#include <QDateTime>
#include <QLineEdit>
#include <QRegularExpression>
#include <QRegularExpressionValidator>
#include <QTimer>
#include <algorithm>
#include <utility>

namespace ikea400::utils {
inline void startSyncedTimer(QTimer* timer) {
  // 1. Calculate time until next whole second
  qint64 currentMsecs = QDateTime::currentMSecsSinceEpoch();
  qint64 remainder = 1000 - (currentMsecs % 1000);

  // 2. Schedule the recurring timer to start precisely at the boundary
  QTimer::singleShot(static_cast<int>(remainder), [timer]() {
    // Now we are at the start of the second
    timer->start(1000);
  });
}

template <typename Callable>
inline void startSyncedTimer(Callable&& callback) {
  // 1. Calculate time until next whole second
  qint64 currentMsecs = QDateTime::currentMSecsSinceEpoch();
  qint64 remainder = 1000 - (currentMsecs % 1000);

  // 2. Schedule the recurring timer to start precisely at the boundary
  QTimer::singleShot(static_cast<int>(remainder),
                     [callback = std::move(callback)]() {
                       // Now we are at the start of the second
                       callback();
                     });
}

inline void setupBase32Input(QLineEdit* lineEdit) {
  // Regex explanation:
  // [A-Za-z2-7]* : Allows any number of these characters
  QRegularExpression regex("^[A-Za-z2-7]*$");
  auto* validator = new QRegularExpressionValidator(regex, lineEdit);

  lineEdit->setValidator(validator);

  // Optional: Force uppercase automatically to handle user typing
  QObject::connect(lineEdit, &QLineEdit::textChanged,
                   [lineEdit](const QString& text) {
                     QString upperText = text.toUpper();
                     if (text != upperText) {
                       lineEdit->setText(upperText);
                     }
                   });
}
}  // namespace ikea400::utils