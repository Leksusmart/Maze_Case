#pragma once

#include "MazeWindow.h"

#include <QCloseEvent>
#include <QDialog>
#include <QHBoxLayout>
#include <QLabel>
#include <QPainter>
#include <QPropertyAnimation>
#include <QPushButton>
#include <QScrollBar>
#include <QTimer>
#include <qboxlayout.h>
#include <qpropertyanimation.h>

namespace Ui { class CaseOpenDialog; }

class CaseOpenDialog : public QDialog
{
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
