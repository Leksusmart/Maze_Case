#include "WelcomeWindow.h"
#include <QHBoxLayout>
#include <QLabel>
#include <QPixmap>
#include <QPropertyAnimation>
#include <QTimer>
#include "./ui_WelcomeWindow.h"

WelcomeWindow::WelcomeWindow(
   QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::WelcomeWindow)
{
   ui->setupUi(this);

   // Задний фон для WelcomeWindow
   QWidget *container = new QWidget(this);
   QHBoxLayout *hLayout = new QHBoxLayout(container);
   QPixmap skyPixmap(":/image/Background1.png"); // Изображение для заднего фона WelcomeWindow
   skyPixmap = skyPixmap.scaledToHeight(this->height(), Qt::SmoothTransformation);
   QLabel *skyLabel1 = new QLabel(container);
   QLabel *skyLabel2 = new QLabel(container);
   skyLabel1->setPixmap(skyPixmap);
   skyLabel2->setPixmap(skyPixmap);
   skyLabel1->setFixedSize(skyPixmap.size());
   skyLabel2->setFixedSize(skyPixmap.size());

   hLayout->addWidget(skyLabel1);
   hLayout->addWidget(skyLabel2);
   hLayout->setSpacing(0);
   hLayout->setContentsMargins(0, 0, 0, 0);

   container->setLayout(hLayout);
   container->setGeometry(0, 0, skyPixmap.width() * 2, this->height());
   container->lower();

   QTimer *timer = new QTimer(this);
   connect(timer, &QTimer::timeout, this, [this, skyLabel1, skyLabel2, skyPixmap]() {
      if (skyLabel1->pos().x() <= -skyPixmap.width() / 2) {
         skyLabel1->move(skyLabel2->pos().x() + skyPixmap.width(), 0);
      } else if (skyLabel2->pos().x() <= -skyPixmap.width() / 2) {
         skyLabel2->move(skyLabel1->pos().x() + skyPixmap.width(), 0);
      }
   });
   timer->start(1000);
   QPropertyAnimation *animation = new QPropertyAnimation(container, "pos");
   animation->setDuration(25000);
   animation->setStartValue(QPoint(0, 0));
   animation->setEndValue(QPoint(-skyPixmap.width(), 0));
   animation->setLoopCount(-1);
   animation->start();
}

WelcomeWindow::~WelcomeWindow()
{
   delete ui;
}
