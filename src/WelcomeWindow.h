#pragma once

#include <QDialog>

namespace Ui { class WelcomeWindow; }

class WelcomeWindow : public QDialog
{
public:
   WelcomeWindow(QWidget *parent = nullptr);
   ~WelcomeWindow();

private:
   Ui::WelcomeWindow *ui;
};
