#include "CaseOpenDialog.h"
#include <QCloseEvent>
#include <QHBoxLayout>
#include <QLabel>
#include "ItemInfoDialog.h"
#include "MazeWindow.h"
#include "ui_CaseOpenDialog.h"
#include "ui_MazeWindow.h"

#include <QPropertyAnimation>
#include <QPushButton>
#include <QScrollBar>
#include <QTimer>

CaseOpenDialog::CaseOpenDialog(
   MazeWindow *parent, int Case_Index)
   : QDialog(parent)
   , parent(parent)
   , Case_Index(Case_Index)
   , ui(new Ui::CaseOpenDialog)
{
   ui->setupUi(this);
   animationTimer = new QTimer(this);
   this->setWindowIcon(QPixmap(parent->ItemPixmaps[Case_Index - 1]));
   switch (Case_Index) {
   case 1:
      this->setWindowTitle(parent->ui->label_Item1_Name->text().left(parent->ui->label_Item1_Name->text().length() - 9));
      break;
   case 2:
      this->setWindowTitle(parent->ui->label_Item2_Name->text().left(parent->ui->label_Item2_Name->text().length() - 9));
      break;
   case 3:
      this->setWindowTitle(parent->ui->label_Item3_Name->text().left(parent->ui->label_Item3_Name->text().length() - 9));
      break;
   default:
      qDebug() << "кейса с индексом " << Case_Index << " не существует";
      return;
   }
   CreateCase();
   connect(ui->Button_Open, &QPushButton::clicked, this, &CaseOpenDialog::StartAnimation);
}

void CaseOpenDialog::CreateCase()
{
   // Эта функция создает линию для анимации.
   if (ui->scrollArea->widget())
      delete ui->scrollArea->widget();
   ui->scrollArea->horizontalScrollBar()->setValue(0);
   QWidget *container = new QWidget();
   QHBoxLayout *hLayout = new QHBoxLayout(container);
   hLayout->setContentsMargins(0, 2, 0, 2);

   // Определяем предметы
   QString begin = parent->ItemPixmaps[Case_Index - 1];
   begin.chop(8);
   QVector<QString> ItemsCommon = {
      begin + "Common1.png", // Обычный
      begin + "Common2.png", // Обычный
      begin + "Common3.png", // Обычный
      begin + "Common4.png", // Обычный
      begin + "Common5.png", // Обычный
      begin + "Common6.png", // Обычный
      begin + "Common7.png"  // Обычный
   };
   QVector<QString> ItemsRare = {
      begin + "Rare1.png", // Редкий
      begin + "Rare2.png", // Редкий
      begin + "Rare3.png", // Редкий
      begin + "Rare4.png", // Редкий
      begin + "Rare5.png"  // Редкий
   };
   QVector<QString> ItemsVeryRare = {
      begin + "VeryRare1.png", // Очень Редкий
      begin + "VeryRare2.png", // Очень Редкий
      begin + "VeryRare3.png"  // Очень Редкий
   };
   QVector<QString> ItemsSecret = {
      begin + "Secret1.png", // Тайное
      begin + "Secret2.png"  // Тайное
   };
   QVector<QString> ItemsLegend = {
      begin + "Legend.png",   // Легендарный
      begin + "Legend1.png",  // Легендарный
      begin + "Legend2.png",  // Легендарный
      begin + "Legend3.png",  // Легендарный
      begin + "Legend4.png",  // Легендарный
      begin + "Legend5.png",  // Легендарный
      begin + "Legend6.png",  // Легендарный
      begin + "Legend7.png",  // Легендарный
      begin + "Legend8.png",  // Легендарный
      begin + "Legend9.png",  // Легендарный
      begin + "Legend10.png", // Легендарный
      begin + "Legend11.png", // Легендарный
      begin + "Legend12.png", // Легендарный
      begin + "Legend13.png", // Легендарный
      begin + "Legend14.png", // Легендарный
      begin + "Legend15.png", // Легендарный
      begin + "Legend16.png", // Легендарный
      begin + "Legend17.png", // Легендарный
      begin + "Legend18.png", // Легендарный
      begin + "Legend19.png", // Легендарный
      begin + "Legend20.png", // Легендарный
      begin + "Legend21.png", // Легендарный
      begin + "Legend22.png", // Легендарный
      begin + "Legend23.png", // Легендарный
      begin + "Legend24.png", // Легендарный
      begin + "Legend25.png", // Легендарный
      begin + "Legend26.png", // Легендарный
      begin + "Legend27.png", // Легендарный
      begin + "Legend28.png", // Легендарный
      begin + "Legend29.png", // Легендарный
      begin + "Legend30.png"  // Легендарный
   };

   // Добавление изображений с учетом вероятностей
   for (int i = 0; i < 35; ++i) {
      QLabel *label = new QLabel();
      float probability = parent->randomGenerator->bounded(1.0);
      QString temp;
      while (true) {
         if (probability <= 0.0026) {
            temp = ItemsLegend[0];
         } else if (probability <= 0.0064) {
            temp = ItemsSecret[parent->randomGenerator->bounded(ItemsSecret.size())];
         } else if (probability <= 0.032) {
            temp = ItemsVeryRare[parent->randomGenerator->bounded(ItemsVeryRare.size())];
         } else if (probability <= 0.16) {
            temp = ItemsRare[parent->randomGenerator->bounded(ItemsRare.size())];
         } else {
            temp = ItemsCommon[parent->randomGenerator->bounded(ItemsCommon.size())];
         }

         // Проверка существования изображения в ресурсах
         QPixmap pixmap(temp);
         if (!pixmap.isNull()) {
            break;
         }
         probability = parent->randomGenerator->bounded(1.0);
      }
      label->setPixmap(QPixmap(temp));
      label->setFixedSize(164, 125);
      label->setScaledContents(true);
      hLayout->addWidget(label);
      if (i == 31) { //выяснено техническим путём
         if (ItemsLegend.contains(temp)) {
            do {
               temp = ItemsLegend[parent->randomGenerator->bounded(1, ItemsLegend.size())];
               QPixmap pixmap(temp);
               if (!pixmap.isNull()) {
                  break;
               }
            } while (true);
         }
         reward = temp;
      }
   }

   container->setLayout(hLayout);
   ui->scrollArea->setWidget(container);
}
void CaseOpenDialog::StartAnimation()
{
   ui->Button_Open->hide();

   QWidget *container = ui->scrollArea->widget();
   animation = new QPropertyAnimation(container, "pos");
   QPoint endPos(0 - ((164 + 6) * 30) + 6 + parent->randomGenerator->bounded(0, 165), 0);
   animation->setEndValue(endPos);

   animation->setDuration(10000);
   animation->setEasingCurve(QEasingCurve::OutCirc);
   animation->start();
   connect(animationTimer, &QTimer::timeout, this, [this]() {
      animationTimer->deleteLater();
      animationTimer = nullptr;
      MazeWindow::item *rez = new MazeWindow::item;
      do {
         if (parent->itemDetails.contains(reward)) {
            rez->photo = reward;
            rez->name = parent->itemDetails[reward].first;
            Float = parent->randomGenerator->bounded(1.0);
            rez->Float = Float;
            rez->isCase = false;
            if (Float >= 0.45)
               rez->cost = parent->itemDetails[reward].second[4].toInt();
            else if (Float >= 0.37)
               rez->cost = parent->itemDetails[reward].second[3].toInt();
            else if (Float >= 0.15)
               rez->cost = parent->itemDetails[reward].second[2].toInt();
            else if (Float >= 0.07)
               rez->cost = parent->itemDetails[reward].second[1].toInt();
            else
               rez->cost = parent->itemDetails[reward].second[0].toInt();
         } else {
            rez->name = "Неизвестно";
            rez->isCase = false;
            rez->Float = -1;
            rez->cost = 0;
         }
      } while (rez->cost == -1);
      QString OldBalance = parent->ui->label_Balance->text();
      ItemInfoDialog *s = new ItemInfoDialog(parent, *rez);

      int result = s->exec();
      bool sold = s->sold;
      QString NewBalance = parent->ui->label_Balance->text();
      if (result == QDialog::Rejected && sold == false)
         parent->putInventory(reward, rez->name, rez->isCase, rez->cost, rez->Float);
      delete s;
      animation->deleteLater();
      this->reject();
   });
   animationTimer->start(10000);
}
void CaseOpenDialog::closeEvent(QCloseEvent *event)
{
   if (animationTimer != nullptr) {
      animationTimer->stop();
      MazeWindow::item *rez = new MazeWindow::item;
      do {
         if (parent->itemDetails.contains(reward)) {
            rez->photo = reward;
            rez->name = parent->itemDetails[reward].first;
            Float = parent->randomGenerator->bounded(1.0);
            rez->Float = Float;
            rez->isCase = false;
            if (Float >= 0.45)
               rez->cost = parent->itemDetails[reward].second[4].toInt();
            else if (Float >= 0.37)
               rez->cost = parent->itemDetails[reward].second[3].toInt();
            else if (Float >= 0.15)
               rez->cost = parent->itemDetails[reward].second[2].toInt();
            else if (Float >= 0.07)
               rez->cost = parent->itemDetails[reward].second[1].toInt();
            else
               rez->cost = parent->itemDetails[reward].second[0].toInt();
         } else {
            rez->name = "Неизвестно";
            rez->isCase = false;
            rez->Float = -1;
            rez->cost = 0;
         }
      } while (rez->cost == -1);
      parent->putInventory(reward, rez->name, rez->isCase, rez->cost, rez->Float);
      this->reject();
   } else {
      animationTimer->deleteLater();
      this->reject();
   }
   event->accept();
}
CaseOpenDialog::~CaseOpenDialog()
{
   animation->deleteLater();
   delete ui;
}
