#include <windows.h>

#include <QtWidgets/QApplication>
#include "raindrop_spectrum.h"
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    raindrop_spectrum w;
    w.show();
    return a.exec();
}
