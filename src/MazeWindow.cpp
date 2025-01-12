#include "MazeWindow.h"
#include "CaseOpenDialog.h"
#include "ItemInfoDialog.h"
#include "ui_MazeWindow.h"

#include <QDebug>
#include <QFile>
#include <QGraphicsOpacityEffect>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLabel>
#include <QMessageBox>
#include <QPropertyAnimation>
#include <QRandomGenerator>
#include <QTimer>

MazeWindow::MazeWindow(QWidget *parent)
   : QMainWindow(parent)
   , ui(new Ui::MazeWindow)
{
   ui->setupUi(this);
   this->setWindowIcon(QIcon("://image/EmptyIcon.png"));

   for (int id = 0; id <= maxId; id++)
      Cell.push_back(cell());
   createMaze();
   this->setFocus();
   loadData();

   ui->label_Finish->setPixmap(QPixmap("://image/Finish.png").scaled(ui->label_Finish->size(), Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation));
   ui->label_Item1->setPixmap(QPixmap(ItemPixmaps[1 - 1]).scaled(ui->label_Item1->size(), Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation));
   ui->label_Item2->setPixmap(QPixmap(ItemPixmaps[2 - 1]).scaled(ui->label_Item2->size(), Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation));
   ui->label_Item3->setPixmap(QPixmap(ItemPixmaps[3 - 1]).scaled(ui->label_Item3->size(), Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation));

   //Подключение кнопок
   connect(ui->pushButton_Play, &QPushButton::clicked, this, [this]() { this->ui->stackedWidget->setCurrentIndex(0); });
   connect(ui->pushButton_Inventory, &QPushButton::clicked, this, [this]() { this->ui->stackedWidget->setCurrentIndex(1); });
   connect(ui->pushButton_Store, &QPushButton::clicked, this, [this]() { this->ui->stackedWidget->setCurrentIndex(2); });
   connect(ui->pushButton_Buy_Item1, &QPushButton::clicked, this, [this]() {
      QString name = ui->label_Item1_Name->text().right(8);
      name.chop(5);
      QString Name = ui->label_Item1_Name->text().left(ui->label_Item1_Name->text().size() - (1 + name.size() + 5));
      int cost = name.toInt();
      if (balanceChange(-cost))
         putInventory(ItemPixmaps[1 - 1], Name, true, cost);
   });
   connect(ui->pushButton_Buy_Item2, &QPushButton::clicked, this, [this]() {
      QString name = ui->label_Item2_Name->text().right(8);
      name.chop(5);
      QString Name = ui->label_Item2_Name->text().left(ui->label_Item2_Name->text().size() - (1 + name.size() + 5));
      int cost = name.toInt();
      if (balanceChange(-cost))
         putInventory(ItemPixmaps[2 - 1], Name, true, cost);
   });
   connect(ui->pushButton_Buy_Item3, &QPushButton::clicked, this, [this]() {
      QString name = ui->label_Item3_Name->text().right(8);
      name.chop(5);
      QString Name = ui->label_Item3_Name->text().left(ui->label_Item3_Name->text().size() - (1 + name.size() + 5));
      int cost = name.toInt();
      if (balanceChange(-cost))
         putInventory(ItemPixmaps[3 - 1], Name, true, cost);
   });
}
void MazeWindow::createData()
{
   qDebug() << "createData: Файл будет пересоздан с значениями по умолчанию";
   data = QJsonObject();
   // Значения по умолчанию
   data["balance"] = "0 руб.";
   data["Finished"] = 0;
   saveData();
}
bool MazeWindow::saveData()
{
   QFile file(filePath);
   if (file.open(QIODevice::WriteOnly)) {
      // Сохранение текущих настроек
      data["balance"] = ui->label_Balance->text();

      // Сохранение инвентаря
      QJsonArray inventoryArray;
      for (const auto &item : Items) {
         QJsonObject itemObject;
         itemObject["name"] = item->name;
         itemObject["photo"] = item->photo;
         itemObject["isCase"] = item->isCase;
         itemObject["cost"] = item->cost;
         itemObject["Float"] = QString::number(item->Float, 'f', 9).toDouble();
         itemObject["index"] = item->index;

         inventoryArray.append(itemObject);
      }
      data["inventory"] = inventoryArray;
      data["Finished"] = QJsonValue::fromVariant(static_cast<unsigned int>(Finished));

      QJsonDocument doc(data);
      file.write(doc.toJson());
      file.close();
      return true;
   } else {
      qDebug() << "saveData: Файл не может быть открыт для записи";
      createData();
      saveData();
      return false;
   }
}
bool MazeWindow::loadData()
{
   QFile file(filePath);
   if (file.open(QIODevice::ReadOnly)) {
      QJsonDocument doc(QJsonDocument::fromJson(file.readAll()));
      data = doc.object();
      file.close();
      //Применение настроек
      bool shouldRecreateFile = false;
      if (data.contains("balance"))
         ui->label_Balance->setText(data["balance"].toString());
      else {
         qDebug() << "loadData: Информация о \"balance\" не найдена";
         shouldRecreateFile = true;
      }
      // Загрузка инвентаря
      if (data.contains("inventory")) {
         QJsonArray inventoryArray = data["inventory"].toArray();
         Items.clear(); // Очистить текущий инвентарь перед загрузкой
         for (const auto &inventoryValue : inventoryArray) {
            QJsonObject item = inventoryValue.toObject();

            QString name = item["name"].toString();
            QString photo = item["photo"].toString();
            bool isCase = item["isCase"].toBool();
            int cost = item["cost"].toInt();
            float Float = item["Float"].toDouble();
            int index = item["index"].toInt();

            putInventory(photo, name, isCase, cost, Float);
         }
      }
      if (data.contains("Finished"))
         Finished = data["Finished"].toInt();
      else {
         qDebug() << "loadData: Информация о \"Finished\" не найдена";
         shouldRecreateFile = true;
      }

      if (shouldRecreateFile) {
         qDebug() << "loadData: Так как в файле не были найдены необходимые значения он считается повреждённым";
         createData();
         return false;
      }
      return true;
   } else {
      qDebug() << "loadData: Файл не может быть открыт для чтения";
      createData();
      return false;
   }
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
   if (index < Items.size()) {
      Items.removeAt(index); // Удаляем элемент из вектора Items
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
   for (int id = 0; id <= maxId; id++) {
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
   ui->Player->move(ui->GroupMaze->width() - step + PlayerOffset, ui->GroupMaze->height() - step + PlayerOffset);
   for (QLabel *Wall : ui->GroupMaze->findChildren<QLabel *>()) {
      if (Wall != ui->Player)
         Wall->deleteLater();
   }
   walls.clear();
   for (int id = 0; id <= maxId; id++) {
      Cell[id].WasThere = false;
      Cell[id].Up = false;
      Cell[id].Left = false;
      Cell[id].Down = false;
      Cell[id].Right = false;
   }

   // Алгоритм создания лабиринта Hunt and kill
   int id = maxId; //Начинаем с ячейки право низ
   look();
   short int counter = 1;
   while (counter <= maxId) {
      bool found = false;
      counter = 0;
      for (id = 0; id <= maxId; id++) {
         if (Cell[id].WasThere) {
            counter++;
            int x1 = (id % step) * step;
            int y1 = (int) (id / step) * step;

            unsigned short int id1 = ((x1 + 20) / step) + y1;
            unsigned short int id2 = ((x1 - 20) / step) + y1;
            unsigned short int id3 = (x1 / step) + y1 + 20;
            unsigned short int id4 = (x1 / step) + y1 - 20;
            if (id1 > maxId)
               id1 = maxId;
            if (id2 > maxId)
               id2 = maxId;
            if (id3 > maxId)
               id3 = maxId;
            if (id4 > maxId)
               id4 = maxId;

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
      ui->Player->setStyleSheet("image: url(:/image/PlayerUp.png);");
      if (ui->Player->y() - PlayerOffset - step >= 0 && !isCollision(ui->Player->x() - PlayerOffset, ui->Player->y() - PlayerOffset - 1, false)) {
         id = ((ui->Player->x() - PlayerOffset) / step) + ui->Player->y() - PlayerOffset;
         ui->Player->move(ui->Player->x(), ui->Player->y() - step);
         newId = ((ui->Player->x() - PlayerOffset) / step) + ui->Player->y() - PlayerOffset;
         createWayMarker(id, newId);
      }
      break;
   case Qt::Key_A:
   case Qt::Key_Left:
      ui->Player->setStyleSheet("image: url(:/image/PlayerLeft.png);");
      if (ui->Player->x() - PlayerOffset - step >= 0 && !isCollision(ui->Player->x() - PlayerOffset - 1, ui->Player->y() - PlayerOffset, true)) {
         id = ((ui->Player->x() - PlayerOffset) / step) + ui->Player->y() - PlayerOffset;
         ui->Player->move(ui->Player->x() - step, ui->Player->y());
         newId = ((ui->Player->x() - PlayerOffset) / step) + ui->Player->y() - PlayerOffset;
         createWayMarker(id, newId);
      }
      break;
   case Qt::Key_S:
   case Qt::Key_Down:
      ui->Player->setStyleSheet("image: url(:/image/PlayerDown.png);");
      if (ui->Player->y() - PlayerOffset + step < ui->GroupMaze->height() && !isCollision(ui->Player->x() - PlayerOffset, ui->Player->y() - PlayerOffset + step - 1, false)) {
         id = ((ui->Player->x() - PlayerOffset) / step) + ui->Player->y() - PlayerOffset;
         ui->Player->move(ui->Player->x(), ui->Player->y() + step);
         newId = ((ui->Player->x() - PlayerOffset) / step) + ui->Player->y() - PlayerOffset;
         createWayMarker(id, newId);
      }
      break;
   case Qt::Key_D:
   case Qt::Key_Right:
      ui->Player->setStyleSheet("image: url(:/image/PlayerRight.png);");
      if (ui->Player->x() - PlayerOffset + step < ui->GroupMaze->width() && !isCollision(ui->Player->x() - PlayerOffset + step - 1, ui->Player->y() - PlayerOffset, true)) {
         id = ((ui->Player->x() - PlayerOffset) / step) + ui->Player->y() - PlayerOffset;
         ui->Player->move(ui->Player->x() + step, ui->Player->y());
         newId = ((ui->Player->x() - PlayerOffset) / step) + ui->Player->y() - PlayerOffset;
         createWayMarker(id, newId);
      }
      break;
   default:

      break;
   }
   ui->Player->raise();
   if (ui->Player->x() - PlayerOffset == ui->label_Finish->x() - (ui->GroupMaze->x() + 1) && ui->Player->y() - PlayerOffset == ui->label_Finish->y() - (ui->GroupMaze->y() + 1)) {
      Finished++;
      QString tempRank = "";
      if (Finished < 10) {
      }
      balanceChange(markers * 5);
      markers = 0;
      ui->label_Finish->setGeometry((ui->GroupMaze->x() + 1) + randomGenerator->bounded(0, 20) * step,
                                    (ui->GroupMaze->y() + 1) + randomGenerator->bounded(0, 20) * step,
                                    ui->label_Finish->width(),
                                    ui->label_Finish->height());
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
   QTimer::singleShot(1600, this, [this, tempCost, value]() {
      tempCost->hide();
      delete tempCost;
      if (value > 0) {
         QString currentBalanceText1 = ui->label_Balance->text();
         currentBalanceText1.chop(5);
         int newBalance = currentBalanceText1.toInt() + value;
         ui->label_Balance->setText(QString::number(newBalance) + " руб.");
      }
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
void MazeWindow::closeEvent(QCloseEvent *event)
{
   saveData();
   qDebug() << "Выход из приложения";
   event->accept();
}
MazeWindow::~MazeWindow()
{
   qDeleteAll(Inventory);
   Inventory.clear();
   qDeleteAll(walls);
   walls.clear();
   qDeleteAll(Items);
   Items.clear();
   for (QLabel *existingWall : ui->GroupMaze->findChildren<QLabel *>()) {
      delete existingWall;
   }
   delete ui;
}
