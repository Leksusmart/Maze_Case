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
   QString reward;
   float Float;
public slots:
   void CreateCase();
   void StartAnimation();

private:
   Ui::CaseOpenDialog *ui;
   QPropertyAnimation *animation;
   MazeWindow *parent;
   int Case_Index;
   void closeEvent(QCloseEvent *event) override;
   QTimer *animationTimer;
};

#endif // CASEOPENDIALOG_H
