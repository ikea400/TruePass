#include <drogon/drogon.h>

#include <iostream>

int main() {
  try {
    // Load config file
    drogon::app().loadConfigFile("config.json");
  } catch (const std::exception &e) {
    // Handle exceptions that may occur during configuration loading
    std::cerr << "Error loading configuration: " << e.what() << std::endl;
    return 1;  // Exit with an error code
  }

  std::cout << "Server is started.\n";

  // Run HTTP framework, the method will block in the internal event loop
  drogon::app().run();

  return 0;
}