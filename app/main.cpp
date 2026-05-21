#include <QApplication>
#include "app/Application.h"

int main(int argc, char* argv[]) {
    QApplication qtApp(argc, argv);
    QApplication::setApplicationName("PomoLyth");
    QApplication::setOrganizationName("PomoLyth");

    Application app;
    if (!app.initialize()) {
        return 1;
    }
    app.show();

    return QApplication::exec();
}
