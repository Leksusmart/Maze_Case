#ifndef CASEOPENDIALOG_H
#define CASEOPENDIALOG_H

#include <QDialog>
#include "MazeWindow.h"
#include <qboxlayout.h>
#include <qpropertyanimation.h>

namespace Ui {
class CaseOpenDialog;
}

class CaseOpenDialog : public QDialog
{
   Q_OBJECT

public:
   explicit CaseOpenDialog(MazeWindow *parent = nullptr, int Case_Index = 1);
   ~CaseOpenDialog();
private slots:
   void closeEvent(QCloseEvent *event) override;
   void StartAnimation();
   void CreateCase();
private:
   QPropertyAnimation *animation;
   QTimer *animationTimer;
   Ui::CaseOpenDialog *ui;
   MazeWindow *parent;
   QString reward;
   int Case_Index;
   float Float;
};

#endif // CASEOPENDIALOG_H
