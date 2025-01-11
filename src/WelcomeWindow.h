#ifndef WELCOMEWINDOW_H
#define WELCOMEWINDOW_H

#include <QDialog>

QT_BEGIN_NAMESPACE
namespace Ui {
class WelcomeWindow;
}
QT_END_NAMESPACE

class WelcomeWindow : public QDialog
{
   Q_OBJECT

public:
   WelcomeWindow(QWidget *parent = nullptr);
   ~WelcomeWindow();

private:
   Ui::WelcomeWindow *ui;
};
#endif // WELCOMEWINDOW_H
