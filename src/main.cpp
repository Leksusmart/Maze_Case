#include "MazeWindow.h"
#include "WelcomeWindow.h"

#include <QApplication>
#include <QDialog>

int main(
   int argc, char *argv[])
{
   QApplication a(argc, argv);
   WelcomeWindow w;
   MazeWindow m;
   if (w.exec() == QDialog::Accepted) {
      m.show();
   } else
      return 0;
   return a.exec();
}
