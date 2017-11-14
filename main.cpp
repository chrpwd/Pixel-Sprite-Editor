/*
 * main.cpp
 * The entrypoint for the program. Creates a View object that will display the GUI.
 *
 * Kira Parker
 * Torin McDonald
 * Melody Chang
 * Christian Purdy
 * Brayden Carlson
 *
 * A7: Sprite Editor Implementation (11/13/17)
*/

#include "view.h"
#include <QApplication>
#include "model.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    Model m;
    View w(m);
    w.show();

    return a.exec();
}
