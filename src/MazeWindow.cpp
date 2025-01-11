#include "MazeWindow.h"
#include "CaseOpenDialog.h"
#include "ItemInfoDialog.h"
#include "ui_MazeWindow.h"

#include <QDebug>
#include <QGraphicsOpacityEffect>
#include <QLabel>
#include <QMessageBox>
#include <QPropertyAnimation>
#include <QRandomGenerator>
#include <QTimer>
#include <QVector>

MazeWindow::MazeWindow(QWidget *parent)
   : QMainWindow(parent)
   , ui(new Ui::MazeWindow)
{
   ui->setupUi(this);
   this->setWindowIcon(QIcon("://image/EmptyIcon.png"));
   for (int id = 0; id < 400; id++)
      Cell.push_back(cell());
   createMaze();
   connect(ui->pushButton_Play, &QPushButton::clicked, this, [this]() { this->ui->stackedWidget->setCurrentIndex(0); });
   connect(ui->pushButton_Inventory, &QPushButton::clicked, this, [this]() { this->ui->stackedWidget->setCurrentIndex(1); });
   connect(ui->pushButton_Store, &QPushButton::clicked, this, [this]() { this->ui->stackedWidget->setCurrentIndex(2); });

   //Магазин
   connect(ui->pushButton_Buy_Item1, &QPushButton::clicked, this, [this]() {
      QString name = this->ui->label_Item1_Name->text().right(8);
      name.chop(5);
      int cost = name.toInt();
      if (balanceChange(-cost))
         putInventory(ItemPixmaps[1 - 1], ui->label_Item1_Name->text(), true, cost);
   });
   connect(ui->pushButton_Buy_Item2, &QPushButton::clicked, this, [this]() {
      QString name = this->ui->label_Item2_Name->text().right(8);
      name.chop(5);
      int cost = name.toInt();
      if (balanceChange(-cost))
         putInventory(ItemPixmaps[2 - 1], ui->label_Item2_Name->text(), true, cost);
   });
   connect(ui->pushButton_Buy_Item3, &QPushButton::clicked, this, [this]() {
      QString name = this->ui->label_Item3_Name->text().right(8);
      name.chop(5);
      int cost = name.toInt();
      if (balanceChange(-cost))
         putInventory(ItemPixmaps[3 - 1], ui->label_Item3_Name->text(), true, cost);
   });
   ui->label_Item1->setPixmap(QPixmap(ItemPixmaps[1 - 1]).scaled(ui->label_Item1->size(), Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation));
   ui->label_Item2->setPixmap(QPixmap(ItemPixmaps[2 - 1]).scaled(ui->label_Item2->size(), Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation));
   ui->label_Item3->setPixmap(QPixmap(ItemPixmaps[3 - 1]).scaled(ui->label_Item3->size(), Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation));
}
void MazeWindow::putInventory(QString photo, QString name, bool isCase, int cost, float Float)
{
   //Эта функция добавляет в начало инвентаря предмет
   QPushButton *itemButton = new QPushButton(ui->scrollArea_Inventory);
   itemButton->setFixedSize(166, 126);
   itemButton->setIcon(QIcon(QPixmap(photo)));
   itemButton->setIconSize(QSize(166, 126));
   if (photo.contains("Legend")) {
      itemButton->setStyleSheet("background-color:black;");
   } else {
      itemButton->setStyleSheet("border:none;");
   }
   Inventory.insert(Inventory.begin(), itemButton);

   item *newitem = new item;
   newitem->photo = photo;
   newitem->name = name;
   newitem->isCase = isCase;
   newitem->Float = Float;
   newitem->cost = cost;
   newitem->index = Inventory.indexOf(itemButton);
   Items.push_back(newitem);

   connect(itemButton, &QPushButton::clicked, [this, itemButton, newitem, photo]() {
      int index = Inventory.indexOf(itemButton);
      ItemInfoDialog w(this, *newitem, index);
      int Case_Index = QString(photo[photo.indexOf("item") + 4]).toInt();
      if (w.exec() == QDialog::Accepted) {
         //Открыть новое окно с опенингом кейса
         CaseOpenDialog *w = new CaseOpenDialog(this, Case_Index);
         w->exec();
      }
   });
   for (int i = 0; i < Inventory.size(); ++i) {
      int row = i / 4;
      int column = i % 4;
      ui->gridLayout->removeWidget(Inventory[i]);
      ui->gridLayout->addWidget(Inventory[i], row, column);
   }
}
void MazeWindow::getInventory(int index)
{
   //Эта функция убирает из инвентаря предмет под индексом
   if (index < 0 || index >= Inventory.size()) {
      return;
   }

   QPushButton *itemButton = Inventory.takeAt(index);
   ui->gridLayout->removeWidget(itemButton);
   itemButton->deleteLater();

   for (int i = index; i < Inventory.size(); ++i) {
      ui->gridLayout->removeWidget(Inventory[i]);
      int newRow = i / 4;
      int newColumn = i % 4;
      ui->gridLayout->addWidget(Inventory[i], newRow, newColumn);
   }
}
void MazeWindow::createWayMarker(short int id1, short int id2)
{
   // Эта функция создаёт между двумя рядом лежащими клетками линию
   int x1 = (id1 % step) * step;
   int y1 = (int) (id1 / step) * step;
   int x2 = (id2 % step) * step;
   int y2 = (int) (id2 / step) * step;

   int way = id1 - id2;
   QLabel *Wall = new QLabel(ui->GroupMaze);
   int x;
   int y;

   switch (way) {
   case -20: // Вниз
      Wall->setStyleSheet("background-color: Silver; border: 0px;");
      Wall->setFixedSize(2, 20);
      x = x1 + 9;
      y = y1 + 11;
      Wall->move(x, y);
      break;
   case -1: // Вправо
      Wall->setStyleSheet("background-color: Silver; border: 0px;");
      Wall->setFixedSize(20, 2);
      x = x1 + 11;
      y = y1 + 9;
      Wall->move(x, y);
      break;
   case 20: // Вверх
      Wall->setStyleSheet("background-color: Silver; border: 0px;");
      Wall->setFixedSize(2, 20);
      x = x1 + 9;
      y = y1 - 20 + 9;
      Wall->move(x, y);
      break;
   case 1: // Влево
      Wall->setStyleSheet("background-color: Silver; border: 0px;");
      Wall->setFixedSize(20, 2);
      x = x1 - 20 + 9;
      y = y1 + 9;
      Wall->move(x, y);
      break;
   default:
      Wall->deleteLater();
      return;
   }

   // Проверка на существующие стены
   int counter = 0;
   for (QLabel *existingWall : ui->GroupMaze->findChildren<QLabel *>()) {
      if (existingWall->geometry().contains(Wall->geometry()) && counter == 1) {
         Wall->deleteLater();
         return;
      } else if (existingWall->geometry().contains(Wall->geometry()))
         counter++;
   }

   Wall->show();
   markers++;
}
void MazeWindow::createWalls()
{
   // Эта функция ставит стены в нужных местах в уже сгенерированном лабиринте
   for (int id = 0; id < 400; id++) {
      int x = (id % step) * step;
      int y = (int) (id / step) * step;
      if (!Cell[id].WasThere)
         continue;
      if (!Cell[id].Up) {
         QLabel *Wall = new QLabel(ui->GroupMaze);
         Wall->setStyleSheet("background-color: black; border: 0px;");
         Wall->setFixedSize(20, 2);
         Wall->move(x, y - 1);
         Wall->show();
         walls.append(Wall);
      }
      if (!Cell[id].Down) {
         QLabel *Wall = new QLabel(ui->GroupMaze);
         Wall->setStyleSheet("background-color: black; border: 0px;");
         Wall->setFixedSize(20, 2);
         Wall->move(x, y + step - 1);
         Wall->show();
         walls.append(Wall);
      }
      if (!Cell[id].Left) {
         QLabel *Wall = new QLabel(ui->GroupMaze);
         Wall->setStyleSheet("background-color: black; border: 0px;");
         Wall->setFixedSize(2, 20);
         Wall->move(x - 1, y);
         Wall->show();
         walls.append(Wall);
      }
      if (!Cell[id].Right) {
         QLabel *Wall = new QLabel(ui->GroupMaze);
         Wall->setStyleSheet("background-color: black; border: 0px;");
         Wall->setFixedSize(2, 20);
         Wall->move(x + step - 1, y);
         Wall->show();
         walls.append(Wall);
      }
   }
}
void MazeWindow::createMaze()
{
   // Эта функция создаёт с помощью алгоритма связи ячеек лабиринта

   // Обнуляем лабиринт
   ui->Player->move(ui->GroupMaze->width() - step, ui->GroupMaze->height() - step);
   for (QLabel *Wall : ui->GroupMaze->findChildren<QLabel *>()) {
      Wall->deleteLater();
   }
   walls.clear();
   for (int id = 0; id < 400; id++) {
      Cell[id].WasThere = false;
      Cell[id].Up = false;
      Cell[id].Left = false;
      Cell[id].Down = false;
      Cell[id].Right = false;
   }

   // Алгоритм создания лабиринта Hunt and kill
   int id = 399; //Начинаем с ячейки право низ
   look();
   short int counter = 1;
   while (counter != 400) {
      bool found = false;
      counter = 0;
      for (id = 0; id < 400; id++) {
         if (Cell[id].WasThere) {
            counter++;
            int x1 = (id % step) * step;
            int y1 = (int) (id / step) * step;

            unsigned short int id1 = ((x1 + 20) / step) + y1;
            unsigned short int id2 = ((x1 - 20) / step) + y1;
            unsigned short int id3 = (x1 / step) + y1 + 20;
            unsigned short int id4 = (x1 / step) + y1 - 20;
            if (id1 > 399)
               id1 = 399;
            if (id2 > 399)
               id2 = 399;
            if (id3 > 399)
               id3 = 399;
            if (id4 > 399)
               id4 = 399;

            if (!Cell[id1].WasThere || !Cell[id2].WasThere || !Cell[id3].WasThere || !Cell[id4].WasThere) {
               look(id);
               found = true;
            }
         }
         if (found) {
            break;
         }
      }
   }
   createWalls();
}
void MazeWindow::look(short int id)
{
   //Эта функция ищет нужную клетку для алгоритма Hunt and Kill
   bool Stuck = false;
   while (!Stuck) {
      int newId;
      Cell[id].WasThere = true;
      int x = (id % step) * step;
      int y = (int) (id / step) * step;

      QVector<short int> AllowedWays = {};

      if (x + step < ui->GroupMaze->width()) { //Можем направо
         int newx = x + step;
         newId = (newx / step) + y;
         if (!Cell[newId].WasThere)
            AllowedWays.push_back(4);
      }
      if (x - step >= 0) { //Можем налево
         int newx = x - step;
         newId = (newx / step) + y;
         if (!Cell[newId].WasThere)
            AllowedWays.push_back(2);
      }
      if (y + step < ui->GroupMaze->height()) { //Можем вниз
         Stuck = false;
         int newy = y + step;
         newId = (x / step) + newy;
         if (!Cell[newId].WasThere)
            AllowedWays.push_back(3);
      }
      if (y - step >= 0) { //Можем вверх
         int newy = y - step;
         newId = (x / step) + newy;
         if (!Cell[newId].WasThere)
            AllowedWays.push_back(1);
      }

      if (AllowedWays.size()) {
         int rand = randomGenerator->bounded(AllowedWays.size());
         switch (AllowedWays[rand]) {
         case 1: //Up
            Cell[id].Up = true;
            y -= step;
            newId = (x / step) + y;
            Cell[newId].Down = true;
            break;
         case 2: //Left
            Cell[id].Left = true;
            x -= step;
            newId = (x / step) + y;
            Cell[newId].Right = true;
            break;
         case 3: //Down
            Cell[id].Down = true;
            y += step;
            newId = (x / step) + y;
            Cell[newId].Up = true;
            break;
         case 4: //Right
            Cell[id].Right = true;
            x += step;
            newId = (x / step) + y;
            Cell[newId].Left = true;
            break;
         }
      } else {
         Stuck = true;
      }
      id = newId;
   }
}
void MazeWindow::keyPressEvent(QKeyEvent *event)
{
   //Эта функия обрабатывает кнопки с клавиатуры для управлением лабиринтом
   short int id;
   short int newId;
   switch (event->key()) {
   case Qt::Key_W:
   case Qt::Key_Up:
      if (ui->Player->y() - step >= 0 && !isCollision(ui->Player->x(), ui->Player->y() - 1, false)) {
         id = (ui->Player->x() / step) + ui->Player->y();
         ui->Player->move(ui->Player->x(), ui->Player->y() - step);
         newId = (ui->Player->x() / step) + ui->Player->y();
         createWayMarker(id, newId);
      }
      break;
   case Qt::Key_A:
   case Qt::Key_Left:
      if (ui->Player->x() - step >= 0 && !isCollision(ui->Player->x() - 1, ui->Player->y(), true)) {
         id = (ui->Player->x() / step) + ui->Player->y();
         ui->Player->move(ui->Player->x() - step, ui->Player->y());
         newId = (ui->Player->x() / step) + ui->Player->y();
         createWayMarker(id, newId);
      }
      break;
   case Qt::Key_S:
   case Qt::Key_Down:
      if (ui->Player->y() + step < ui->GroupMaze->height() && !isCollision(ui->Player->x(), ui->Player->y() + step - 1, false)) {
         id = (ui->Player->x() / step) + ui->Player->y();
         ui->Player->move(ui->Player->x(), ui->Player->y() + step);
         newId = (ui->Player->x() / step) + ui->Player->y();
         createWayMarker(id, newId);
      }
      break;
   case Qt::Key_D:
   case Qt::Key_Right:
      if (ui->Player->x() + step < ui->GroupMaze->width() && !isCollision(ui->Player->x() + step - 1, ui->Player->y(), true)) {
         id = (ui->Player->x() / step) + ui->Player->y();
         ui->Player->move(ui->Player->x() + step, ui->Player->y());
         newId = (ui->Player->x() / step) + ui->Player->y();
         createWayMarker(id, newId);
      }
      break;
   default:

      break;
   }
   if (ui->Player->x() == ui->label_Finish->x() - 201 && ui->Player->y() == ui->label_Finish->y() - 26) {
      balanceChange(markers * 5);
      markers = 0;
      ui->label_Finish->setGeometry(201 + randomGenerator->bounded(0, 20) * step, 26 + randomGenerator->bounded(0, 20) * step, ui->label_Finish->width(), ui->label_Finish->height());
      createMaze();
   }
   QMainWindow::keyPressEvent(event);
}
bool MazeWindow::balanceChange(int value)
{
   QString currentBalanceText = ui->label_Balance->text();
   currentBalanceText.chop(5);
   int newBalance = currentBalanceText.toInt() + value;

   if (newBalance < 0) {
      QMessageBox::warning(this, "Ошибка", "Недостаточно средств");
      return false;
   }

   // Создаем временный QLabel для анимации
   QLabel *tempCost = new QLabel(this);
   tempCost->setFixedSize(ui->label_Balance->size());
   tempCost->setAlignment(Qt::AlignCenter);
   tempCost->setStyleSheet("background-color: transparent; font: 28pt 'Ink Free'; color: rgb(255, 255, 0);");
   tempCost->move(ui->label_Balance->x(), ui->label_Balance->y() + 50);
   QPoint startPos = tempCost->pos();
   QPoint endPos = ui->label_Balance->pos() + QPoint(0, 20);
   if (value < 0) {
      QPoint temp = endPos;
      endPos = startPos;
      startPos = temp;
      tempCost->setText(QString::number(value) + " руб.");
      ui->label_Balance->setText(QString::number(newBalance) + " руб.");
   } else
      tempCost->setText("+" + QString::number(value) + " руб. ");
   tempCost->show();

   // Создаем анимацию для перемещения
   QPropertyAnimation *positionAnimation = new QPropertyAnimation(tempCost, "pos");
   positionAnimation->setStartValue(startPos);
   positionAnimation->setEndValue(endPos);
   positionAnimation->setDuration(1500);
   positionAnimation->setEasingCurve(QEasingCurve::Linear);
   positionAnimation->start();

   // Создаем эффект прозрачности
   QGraphicsOpacityEffect *opacityEffect = new QGraphicsOpacityEffect(tempCost);
   tempCost->setGraphicsEffect(opacityEffect);

   // Создаем анимацию для изменения прозрачности
   QPropertyAnimation *opacityAnimation = new QPropertyAnimation(opacityEffect, "opacity");
   opacityAnimation->setDuration(1500);
   opacityAnimation->setStartValue(1.0);
   opacityAnimation->setEndValue(0.0);
   opacityAnimation->start();

   // Обновляем баланс после завершения анимации
   QTimer::singleShot(1600, this, [this, tempCost, newBalance, value]() {
      tempCost->hide();
      delete tempCost;
      if (value > 0)
         ui->label_Balance->setText(QString::number(newBalance) + " руб.");
   });

   return true;
}
bool MazeWindow::isCollision(int x, int y, bool vertical)
{
   for (QLabel *wall : walls) {
      if (wall->geometry().contains(x, y) && wall->width() > wall->height()) {
         if (!vertical)
            return true;
      } else if (wall->geometry().contains(x, y) && wall->width() < wall->height()) {
         if (vertical)
            return true;
      }
   }
   return false;
}
MazeWindow::~MazeWindow()
{
   qDeleteAll(Inventory);
   Inventory.clear();
   qDeleteAll(walls);
   walls.clear();
   delete ui;
}
