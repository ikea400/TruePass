#pragma once

#include <algorithm>
#include <format>
#include <iostream>
#include <string>
#include <utility>

#include "Timer.h"

namespace ikea400 {

using Printer = void (*)(const std::string&);

class ScopedTimer {
 public:
  explicit ScopedTimer(std::string name, Printer printer = defaultPrinterImpl)
      : m_timer(true), m_name(std::move(name)), m_printer(printer) {}

  ~ScopedTimer() {
    auto elapsed = m_timer.restart<std::chrono::microseconds>();
    m_printer(std::format("[{}] Elapsed time: {} ms", m_name, (elapsed.count() / 1000.0)));
  }

  static void defaultPrinterImpl(const std::string& s) {
    std::cout << s << '\n';
  }

 private:
  std::string m_name;
  Timer m_timer;

  Printer m_printer;
};

}  // namespace ikea400