#pragma once

#include <chrono>

#include "utils.h"

namespace ikea400 {

class Timer {
 public:
  Timer(bool start = false) {
    if (start) {
      this->start();
    }
  }

  void start() { m_startTime = std::chrono::steady_clock::now(); }

  template <typename T = std::chrono::milliseconds>
    requires utils::is_duration_v<T>
  T elapsed() const {
    auto now = std::chrono::steady_clock::now();
    return std::chrono::duration_cast<T>(now - m_startTime);
  }

  template <typename T = std::chrono::milliseconds>
   requires utils::is_duration_v<T>
  T restart() {
    auto now = std::chrono::steady_clock::now();
    auto elapsedTime = std::chrono::duration_cast<T>(now - m_startTime);
    m_startTime = now;
    return elapsedTime;
  }

 private:
  std::chrono::steady_clock::time_point m_startTime;
};
}  // namespace ikea400