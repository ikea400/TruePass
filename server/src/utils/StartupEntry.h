#pragma once
#include <drogon/HttpAppFramework.h>

template <typename T>
struct StartupEntry {
 protected:
  StartupEntry() { (void)registered; }

 private:
  static void registerType() {
    drogon::app().registerBeginningAdvice(
        []() { startup_beginning(static_cast<T*>(nullptr)); });
  }

  static inline bool registered = []() {
    registerType();
    return true;
  }();
};