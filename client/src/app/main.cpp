
#include <QtWidgets/QApplication>

#include "AppController.h"

int main(int argc, char *argv[]) {
  QApplication app(argc, argv);

  app.setOrganizationName("ikea400");
  app.setApplicationName("TruePass");

  AppController appController;
  appController.start();

  return app.exec();
}