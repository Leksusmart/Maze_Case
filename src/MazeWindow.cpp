#include "MazeWindow.h"
#include "CaseOpenDialog.h"
#include "ItemInfoDialog.h"
#include "ui_MazeWindow.h"

#include <QDebug>
#include <QDir>
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

#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
MazeWindow::MazeWindow(QWidget *parent)
   : QMainWindow(parent)
   , ui(new Ui::MazeWindow)
   , networkManager(new QNetworkAccessManager(this))
{
   ui->setupUi(this);

   this->setWindowIcon(QIcon("://image/EmptyIcon.png"));
   ui->label_Finish->setPixmap(QPixmap("://image/Finish.png").scaled(ui->label_Finish->size(), Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation));
   ui->label_Item1->setPixmap(QPixmap(itemDetails[1 - 1].source).scaled(ui->label_Item1->size(), Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation));
   ui->label_Item2->setPixmap(QPixmap(itemDetails[2 - 1].source).scaled(ui->label_Item2->size(), Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation));
   ui->label_Item3->setPixmap(QPixmap(itemDetails[3 - 1].source).scaled(ui->label_Item3->size(), Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation));

   item Case1{itemDetails[1 - 1].name, itemDetails[1 - 1].source, true, -1, itemDetails[1 - 1].cost[0].toDouble()};
   item Case2{itemDetails[2 - 1].name, itemDetails[2 - 1].source, true, -1, itemDetails[2 - 1].cost[0].toDouble()};
   item Case3{itemDetails[3 - 1].name, itemDetails[3 - 1].source, true, -1, itemDetails[3 - 1].cost[0].toDouble()};
   updatePrice(Case1);
   updatePrice(Case2);
   updatePrice(Case3);

   //Подключение кнопок
   connect(ui->pushButton_Play, &QPushButton::clicked, this, [this]() {
      checkRank();
      update();
      ui->stackedWidget->setCurrentIndex(0);
   });
   connect(ui->pushButton_Inventory, &QPushButton::clicked, this, [this]() {
      ui->label_Rank->setPixmap(QPixmap());
      update();
      ui->stackedWidget->setCurrentIndex(1);
   });
   connect(ui->pushButton_Store, &QPushButton::clicked, this, [this]() {
      ui->label_Rank->setPixmap(QPixmap());
      update();
      ui->stackedWidget->setCurrentIndex(2);
   });
   connect(ui->pushButton_Buy_Item1, &QPushButton::clicked, this, [this]() {
      QString name = ui->label_Item1_Name->text().right(8);
      name.chop(5);
      QString Name = ui->label_Item1_Name->text().left(ui->label_Item1_Name->text().size() - (1 + name.size() + 5));
      int cost = name.toInt();
      if (balanceChange(-cost))
         putInventory(itemDetails[1 - 1].source, Name, true, cost);
   });
   connect(ui->pushButton_Buy_Item2, &QPushButton::clicked, this, [this]() {
      QString name = ui->label_Item2_Name->text().right(8);
      name.chop(5);
      QString Name = ui->label_Item2_Name->text().left(ui->label_Item2_Name->text().size() - (1 + name.size() + 5));
      int cost = name.toInt();
      if (balanceChange(-cost))
         putInventory(itemDetails[2 - 1].source, Name, true, cost);
   });
   connect(ui->pushButton_Buy_Item3, &QPushButton::clicked, this, [this]() {
      QString name = ui->label_Item3_Name->text().right(8);
      name.chop(5);
      QString Name = ui->label_Item3_Name->text().left(ui->label_Item3_Name->text().size() - (1 + name.size() + 5));
      int cost = name.toInt();
      if (balanceChange(-cost))
         putInventory(itemDetails[3 - 1].source, Name, true, cost);
   });
   connect(ui->Button_Size, &QPushButton::clicked, this, [this]() {
      // Создание нового лабиринта с выбранным размером + обновление интерфейса
      int newRows = ui->spinBox_SizeV->value();
      int newCols = ui->spinBox_SizeH->value();

      bool needUpdate = true;
      if (ui->GroupMaze->width() / step == newCols && ui->GroupMaze->height() / step == newRows)
         needUpdate = false;

      ui->GroupMaze->setFixedSize(newCols * step, newRows * step);

      createMaze();
      if (needUpdate)
         update();
   });

   loadData();
   update();
   createMaze();
   ui->stackedWidget->setCurrentIndex(0);
}
void MazeWindow::updatePrice(MazeWindow::item &item)
{
   QString ITEM_ID = "";
   int index = 0;
   if (item.Float >= 0.45)
      index = 4;
   else if (item.Float >= 0.37)
      index = 3;
   else if (item.Float >= 0.15)
      index = 2;
   else if (item.Float >= 0.07)
      index = 1;
   else
      index = 0;
   for (int i = 0; i < itemDetails.size(); i++) {
      if (itemDetails[i].name == item.name) {
         ITEM_ID = itemDetails[i].steamID[index];
         break;
      }
   }
   if (ITEM_ID == "")
      return;

   // Ищем элемент в списке
   auto it = std::find_if(itemDetails.begin(), itemDetails.end(), [this, item](MazeWindow::ItemDetail &Item) { return Item.source == item.photo; });

   QString url = QString("https://steamcommunity.com/market/itemordershistogram?&language=english&currency=5&item_nameid=%1").arg(ITEM_ID);
   qDebug() << url;
   QNetworkReply *reply = networkManager->get(QNetworkRequest(QUrl(url)));

   connect(reply, &QNetworkReply::finished, [this, reply, &item, index, it]() {
      if (reply->error() == QNetworkReply::NoError) {
         QByteArray responseData = reply->readAll();
         QString jsonString = QString::fromUtf8(responseData);

         QJsonDocument jsonDoc = QJsonDocument::fromJson(jsonString.toUtf8());
         QJsonObject jsonObj = jsonDoc.object();

         double cost = 0.0;
         if (jsonObj["lowest_sell_order"].toString() != "") {
            cost = jsonObj["lowest_sell_order"].toString().toDouble();
            cost /= 100.0;
         } else {
            QJsonArray buyOrderGraph = jsonObj["buy_order_graph"].toArray();
            QJsonArray firstOrder = buyOrderGraph[0].toArray();
            cost = firstOrder[0].toDouble();
         }

         qDebug() << QString("Cost = %1").arg(cost, 0, 'f', 2);

         item.cost = cost;
         it->cost[index] = QString("%1").arg(cost, 0, 'f', 2);
         if (it->source == itemDetails[1 - 1].source)
            ui->label_Item1_Name->setText(QString("%1 %2 руб.").arg(itemDetails[1 - 1].name, itemDetails[1 - 1].cost[0]));
         else if (it->source == itemDetails[2 - 1].source)
            ui->label_Item2_Name->setText(QString("%1 %2 руб.").arg(itemDetails[2 - 1].name, itemDetails[2 - 1].cost[0]));
         else if (it->source == itemDetails[3 - 1].source)
            ui->label_Item3_Name->setText(QString("%1 %2 руб.").arg(itemDetails[3 - 1].name, itemDetails[3 - 1].cost[0]));
      } else {
         item.cost = it->cost[index].toDouble();
         qDebug() << "Ошибка при получении данных:" << reply->errorString();
      }
      reply->deleteLater();
   });
}
void MazeWindow::update()
{
   setFocus();

   // Вычесление максимальных значений
   QSize screen = this->screen()->size();
   maxCols = (int) (screen.width() - (6 + ui->labelStatic_Hint->width() + 6 + 6 + ui->label_Size->width() + 6)) / step;
   maxRows = (int) ((screen.height() - 77) - (5 + ui->label_Balance->height() + ui->pushButton_Inventory->height())) / step;
   ui->label_Size->setText(QString("Размер лабиринта\nmax x = %1\nmax y = %2").arg(maxCols).arg(maxRows));
   ui->spinBox_SizeH->setMaximum(maxCols);
   ui->spinBox_SizeV->setMaximum(maxRows);
   // Обновление лабиринта
   int width = ui->GroupMaze->width();
   int height = ui->GroupMaze->height();
   int oldx = ui->GroupMaze->x();
   int oldy = ui->GroupMaze->y();
   ui->GroupMaze->move(QPoint(1, 0));
   ui->label_Finish->move(ui->label_Finish->x() + ui->GroupMaze->x() - oldx, ui->label_Finish->y() + ui->GroupMaze->y() - oldy);
   if (ui->label_Finish->x() >= ui->GroupMaze->width() || ui->label_Finish->y() >= ui->GroupMaze->height())
      ui->label_Finish->move(ui->GroupMaze->x() + 1, ui->GroupMaze->y() + 1);

   // Обновление рамки лабиринта
   int x = ui->GroupMaze->x() - 1;
   int y = ui->GroupMaze->y();
   ui->labelStatic_Wall_Left->setGeometry(x, y, 2, height + 1);
   ui->labelStatic_Wall_Right->setGeometry(x + width, y, 2, height + 1);
   ui->labelStatic_Wall_Up->setGeometry(x, y, width + 2, 2);
   ui->labelStatic_Wall_Down->setGeometry(x, y + height - 1, width + 2, 2);
   ui->labelStatic_Wall_Left->raise();
   ui->labelStatic_Wall_Right->raise();
   ui->labelStatic_Wall_Up->raise();
   ui->labelStatic_Wall_Down->raise();

   // Ограничение окна
   ui->groupBox->setMinimumSize(ui->GroupMaze->size() + QSize(2, 1));
   ui->stackedWidget->setMinimumSize(ui->groupBox->width(), ui->groupBox->height());
   ui->scrollArea_Inventory->resize(ui->stackedWidget->size());
   ui->scrollArea_Store->resize(ui->stackedWidget->size());
   this->setMinimumWidth(6 + ui->labelStatic_Hint->width() + 6 + ui->groupBox->width() + 6 + ui->label_Size->width() + 6);
   this->setMinimumHeight(5 + ui->groupBox->height() + ui->label_Balance->height() + ui->pushButton_Inventory->height());
}
void MazeWindow::resizeEvent(QResizeEvent *event)
{
   QMainWindow::resizeEvent(event);
   ui->scrollArea_Inventory->resize(ui->stackedWidget->size());
   ui->scrollArea_Store->resize(ui->stackedWidget->size());

   // Инвентарь
   for (int i = 0; i < Inventory.size(); ++i) {
      int row = i / (int) ((this->width() + 5) / (166 + 15));
      int column = i % (int) ((this->width() + 5) / (166 + 15));
      ui->gridLayout->removeWidget(Inventory[i]);
      ui->gridLayout->addWidget(Inventory[i], row, column);
   }
   // Всё остальное в функции
   update();
}
void MazeWindow::createData()
{
   qDebug() << "createData: Файл будет пересоздан с значениями по умолчанию";
   data = QJsonObject();
   // Значения по умолчанию
   data["balance"] = "0 руб.";
   data["score"] = 0;
   saveData();
}
bool MazeWindow::saveData()
{
   QDir dir(QFileInfo(filePath).absolutePath());

   if (!dir.exists()) {
      if (!dir.mkpath(".")) {
         qDebug() << "saveData: Не удалось создать директории по пути" << dir.absolutePath();
         return false;
      }
   }

   QFile file(filePath);
   if (!file.open(QIODevice::WriteOnly)) {
      qDebug() << "saveData: Файл не может быть открыт для записи";
      createData();
      return false;
   }

   // Сохранение текущих настроек
   data["balance"] = ui->label_Balance->text();
   data["score"] = QJsonValue::fromVariant(static_cast<double>(score));

   // Сохранение инвентаря
   QJsonArray inventoryArray;
   for (const auto &item : Items) {
      QJsonObject itemObject;

      itemObject["name"] = item->name;
      itemObject["photo"] = item->photo;
      itemObject["isCase"] = item->isCase;
      itemObject["cost"] = item->cost;
      if (!item->isCase) {
         itemObject["Float"] = QString::number(item->Float, 'f', 9).toDouble();
      }

      inventoryArray.append(itemObject);
   }
   data["inventory"] = inventoryArray;

   QJsonDocument doc(data);
   qint64 bytesWritten = file.write(doc.toJson());
   file.close();

   if (bytesWritten == -1) {
      qDebug() << "saveData: Количество байт в файле не соответствует тому что мы записали";
      return false;
   }

   return true;
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

            bool isCase = item["isCase"].toBool();
            QString name = item["name"].toString();
            QString photo = item["photo"].toString();
            int cost = item["cost"].toInt();
            if (isCase) {
               putInventory(photo, name, isCase, cost);
            } else {
               float Float = item["Float"].toDouble();
               putInventory(photo, name, isCase, cost, Float);
            }
         }
      }
      if (data.contains("score"))
         score = data["score"].toDouble();
      else {
         qDebug() << "loadData: Информация о \"score\" не найдена";
         shouldRecreateFile = true;
      }
      checkRank();

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
   Items.push_back(newitem);

   connect(itemButton, &QPushButton::clicked, [this, itemButton, newitem, photo]() {
      int index = Inventory.indexOf(itemButton);
      ItemInfoDialog a(this, *newitem, index);
      int Case_Index = QString(photo[photo.indexOf("item") + 4]).toInt();
      if (a.exec() == QDialog::Accepted) {
         //Открыть новое окно с опенингом кейса
         CaseOpenDialog *b = new CaseOpenDialog(this, Case_Index);
         b->exec();
         b->deleteLater();
      }
      update();
   });
   for (int i = 0; i < Inventory.size(); ++i) {
      int row = i / (int) ((this->width() + 5) / (166 + 15));
      int column = i % (int) ((this->width() + 5) / (166 + 15));
      ui->gridLayout->removeWidget(Inventory[i]);
      ui->gridLayout->addWidget(Inventory[i], row, column);
   }
   update();
}
void MazeWindow::getInventory(int index)
{
   // Эта функция убирает из инвентаря предмет под индексом
   if (index < 0 || index >= Inventory.size()) {
      return;
   }

   // Удаляем элемент из вектора Items
   if (index < Items.size()) {
      Items.removeAt((Items.size() - 1) - index);
   }

   // Удаляем предмет из инвентаря
   QPushButton *itemButton = Inventory.takeAt(index);
   ui->gridLayout->removeWidget(itemButton);
   itemButton->deleteLater();

   // Обновляем расположение оставшихся предметов в инвентаре
   for (int i = index; i < Inventory.size(); ++i) {
      ui->gridLayout->removeWidget(Inventory[i]);
      int newRow = i / (int) ((width() + 5) / (166 + 15));
      int newColumn = i % (int) ((width() + 5) / (166 + 15));
      ui->gridLayout->addWidget(Inventory[i], newRow, newColumn);
   }
}
void MazeWindow::createWayMarker(short int id1, short int id2)
{
   // Эта функция создаёт между двумя рядом лежащими клетками линию

   int x1 = (id1 % (ui->GroupMaze->width() / step)) * step;
   int y1 = (int) (id1 / (ui->GroupMaze->width() / step)) * step;
   int x2 = (id2 % (ui->GroupMaze->width() / step)) * step;
   int y2 = (int) (id2 / (ui->GroupMaze->width() / step)) * step;

   int way = id1 - id2;
   QLabel *Wall = new QLabel(ui->GroupMaze);
   int x;
   int y;

   if (way == -(ui->GroupMaze->width() / step)) { // Вниз
      Wall->setStyleSheet("background-color: Silver; border: 0px;");
      Wall->setFixedSize(2, 20);
      x = x1 + 9;
      y = y1 + 11;
      Wall->move(x, y);
   } else if (way == -1) { // Вправо
      Wall->setStyleSheet("background-color: Silver; border: 0px;");
      Wall->setFixedSize(20, 2);
      x = x1 + 11;
      y = y1 + 9;
      Wall->move(x, y);
   } else if (way == ui->GroupMaze->width() / step) { // Вверх
      Wall->setStyleSheet("background-color: Silver; border: 0px;");
      Wall->setFixedSize(2, 20);
      x = x1 + 9;
      y = y1 - 20 + 9;
      Wall->move(x, y);
   } else if (way == 1) { // Влево
      Wall->setStyleSheet("background-color: Silver; border: 0px;");
      Wall->setFixedSize(20, 2);
      x = x1 - 20 + 9;
      y = y1 + 9;
      Wall->move(x, y);
   } else {
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
      int x = (id % (ui->GroupMaze->width() / step)) * step;
      int y = (int) (id / (ui->GroupMaze->width() / step)) * step;
      if (!Cell[id].WasThere)
         continue;
      if (!Cell[id].Up && !isCollision(x, y - 1, false)) {
         QLabel *Wall = new QLabel(ui->GroupMaze);
         Wall->setStyleSheet("background-color: black; border: 0px;");
         Wall->setFixedSize(20, 2);
         Wall->move(x, y - 1);
         Wall->show();
         walls.append(Wall);
      }
      if (!Cell[id].Down && !isCollision(x, y + step - 1, false)) {
         QLabel *Wall = new QLabel(ui->GroupMaze);
         Wall->setStyleSheet("background-color: black; border: 0px;");
         Wall->setFixedSize(20, 2);
         Wall->move(x, y + step - 1);
         Wall->show();
         walls.append(Wall);
      }
      if (!Cell[id].Left && !isCollision(x - 1, y, true)) {
         QLabel *Wall = new QLabel(ui->GroupMaze);
         Wall->setStyleSheet("background-color: black; border: 0px;");
         Wall->setFixedSize(2, 20);
         Wall->move(x - 1, y);
         Wall->show();
         walls.append(Wall);
      }
      if (!Cell[id].Right && !isCollision(x + step - 1, y, true)) {
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
   //Эта функция ищет нужную клетку для функции Kill
   maxId = (ui->GroupMaze->height() / step - 1) * (ui->GroupMaze->width() / step) + (ui->GroupMaze->width() / step - 1);
   // Обнуляем лабиринт
   ui->Player->move(ui->GroupMaze->width() - step + PlayerOffset, ui->GroupMaze->height() - step + PlayerOffset);
   for (QLabel *Wall : ui->GroupMaze->findChildren<QLabel *>()) {
      if (Wall != ui->Player)
         Wall->deleteLater();
   }
   walls.clear();
   Cell.clear();
   for (int id = 0; id < maxId; id++) {
      cell tempCell = cell();
      tempCell.WasThere = false;
      tempCell.Up = false;
      tempCell.Left = false;
      tempCell.Down = false;
      tempCell.Right = false;
      Cell.push_back(tempCell);
   }
   // Алгоритм создания лабиринта Hunt and kill
   kill(maxId); // Запуск с стартовой точки
   // Hunt
   short int counter = 1;
   errorCounter = 0;
   while (counter <= maxId) {
      errorCounter++;
      if (errorCounter > maxCols * maxRows) {
         qDebug() << "Error: Бесконечный цикл while в функции createMaze";
         break;
      }
      bool found = false;
      counter = 0;
      for (int id = 0; id <= maxId; id++) {
         if (Cell[id].WasThere) {
            counter++;
            int x1 = (id % (ui->GroupMaze->width() / step)) * step;
            int y1 = (int) (id / (ui->GroupMaze->width() / step)) * step;

            unsigned short int id1 = (y1 / step) * (ui->GroupMaze->width() / step) + ((x1 + step) / step);
            unsigned short int id2 = (y1 / step) * (ui->GroupMaze->width() / step) + ((x1 - step) / step);
            unsigned short int id3 = ((y1 + step) / step) * (ui->GroupMaze->width() / step) + (x1 / step);
            unsigned short int id4 = ((y1 - step) / step) * (ui->GroupMaze->width() / step) + (x1 / step);

            if (id1 > maxId)
               id1 = maxId;
            if (id2 > maxId)
               id2 = maxId;
            if (id3 > maxId)
               id3 = maxId;
            if (id4 > maxId)
               id4 = maxId;

            if (!Cell[id1].WasThere || !Cell[id2].WasThere || !Cell[id3].WasThere || !Cell[id4].WasThere) {
               kill(id);
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
void MazeWindow::kill(short int id)
{
   // Эта функция прокладывает рандомный маршрут с полученой точки лабиринта
   bool Stuck = false;
   errorCounter = 0;
   while (!Stuck) {
      errorCounter++;
      if (errorCounter > maxCols * maxRows) {
         qDebug() << "Error: Бесконечный цикл while в функции kill";
         break;
      }
      int newId;
      Cell[id].WasThere = true;
      int x = (id % (ui->GroupMaze->width() / step)) * step;
      int y = (int) (id / (ui->GroupMaze->width() / step)) * step;

      QVector<short int> AllowedWays = {};

      if (x + step < ui->GroupMaze->width()) { //Можем направо
         int newx = x + step;
         newId = (y / step) * (ui->GroupMaze->width() / step) + (newx / step);
         if (!Cell[newId].WasThere)
            AllowedWays.push_back(4);
      }
      if (x - step >= 0) { //Можем налево
         int newx = x - step;
         newId = (y / step) * (ui->GroupMaze->width() / step) + (newx / step);
         if (!Cell[newId].WasThere)
            AllowedWays.push_back(2);
      }
      if (y + step < ui->GroupMaze->height()) { //Можем вниз
         Stuck = false;
         int newy = y + step;
         newId = (newy / step) * (ui->GroupMaze->width() / step) + (x / step);
         if (!Cell[newId].WasThere)
            AllowedWays.push_back(3);
      }
      if (y - step >= 0) { //Можем вверх
         int newy = y - step;
         newId = (newy / step) * (ui->GroupMaze->width() / step) + (x / step);
         if (!Cell[newId].WasThere)
            AllowedWays.push_back(1);
      }

      if (AllowedWays.size()) {
         int rand = randomGenerator->bounded(AllowedWays.size());
         switch (AllowedWays[rand]) {
         case 1: //Up
            Cell[id].Up = true;
            y -= step;
            newId = (y / step) * (ui->GroupMaze->width() / step) + (x / step);
            Cell[newId].Down = true;
            break;
         case 2: //Left
            Cell[id].Left = true;
            x -= step;
            newId = (y / step) * (ui->GroupMaze->width() / step) + (x / step);
            Cell[newId].Right = true;
            break;
         case 3: //Down
            Cell[id].Down = true;
            y += step;
            newId = (y / step) * (ui->GroupMaze->width() / step) + (x / step);
            Cell[newId].Up = true;
            break;
         case 4: //Right
            Cell[id].Right = true;
            x += step;
            newId = (y / step) * (ui->GroupMaze->width() / step) + (x / step);
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
         id = ((ui->Player->y() - PlayerOffset) / step) * (ui->GroupMaze->width() / step) + ((ui->Player->x() - PlayerOffset) / step);
         ui->Player->move(ui->Player->x(), ui->Player->y() - step);
         newId = ((ui->Player->y() - PlayerOffset) / step) * (ui->GroupMaze->width() / step) + ((ui->Player->x() - PlayerOffset) / step);
         createWayMarker(id, newId);
      }
      break;
   case Qt::Key_A:
   case Qt::Key_Left:
      ui->Player->setStyleSheet("image: url(:/image/PlayerLeft.png);");
      if (ui->Player->x() - PlayerOffset - step >= 0 && !isCollision(ui->Player->x() - PlayerOffset - 1, ui->Player->y() - PlayerOffset, true)) {
         id = ((ui->Player->y() - PlayerOffset) / step) * (ui->GroupMaze->width() / step) + ((ui->Player->x() - PlayerOffset) / step);
         ui->Player->move(ui->Player->x() - step, ui->Player->y());
         newId = ((ui->Player->y() - PlayerOffset) / step) * (ui->GroupMaze->width() / step) + ((ui->Player->x() - PlayerOffset) / step);
         createWayMarker(id, newId);
      }
      break;
   case Qt::Key_S:
   case Qt::Key_Down:
      ui->Player->setStyleSheet("image: url(:/image/PlayerDown.png);");
      if (ui->Player->y() - PlayerOffset + step < ui->GroupMaze->height() && !isCollision(ui->Player->x() - PlayerOffset, ui->Player->y() - PlayerOffset + step - 1, false)) {
         id = ((ui->Player->y() - PlayerOffset) / step) * (ui->GroupMaze->width() / step) + ((ui->Player->x() - PlayerOffset) / step);
         ui->Player->move(ui->Player->x(), ui->Player->y() + step);
         newId = ((ui->Player->y() - PlayerOffset) / step) * (ui->GroupMaze->width() / step) + ((ui->Player->x() - PlayerOffset) / step);
         createWayMarker(id, newId);
      }
      break;
   case Qt::Key_D:
   case Qt::Key_Right:
      ui->Player->setStyleSheet("image: url(:/image/PlayerRight.png);");
      if (ui->Player->x() - PlayerOffset + step < ui->GroupMaze->width() && !isCollision(ui->Player->x() - PlayerOffset + step - 1, ui->Player->y() - PlayerOffset, true)) {
         id = ((ui->Player->y() - PlayerOffset) / step) * (ui->GroupMaze->width() / step) + ((ui->Player->x() - PlayerOffset) / step);
         ui->Player->move(ui->Player->x() + step, ui->Player->y());
         newId = ((ui->Player->y() - PlayerOffset) / step) * (ui->GroupMaze->width() / step) + ((ui->Player->x() - PlayerOffset) / step);
         createWayMarker(id, newId);
      }
      break;
   default:

      break;
   }
   ui->Player->raise();
   if (ui->Player->pos() - QPoint(PlayerOffset, PlayerOffset) == ui->label_Finish->pos() - (ui->GroupMaze->pos() + QPoint(1, 1))) {
      score += (double) ((ui->GroupMaze->width() / step) * (ui->GroupMaze->height() / step)) / (double) (step * step);
      errorCounter = 0;
      do {
         errorCounter++;
         if (errorCounter > 100) {
            qDebug() << "Error: Бесконечный цикл while в функции keyPressEvent";
            break;
         }
         ui->label_Finish->move((ui->GroupMaze->x() + 1) + (randomGenerator->bounded(0, (ui->GroupMaze->width() / step)) * step),
                                (ui->GroupMaze->y() + 1) + (randomGenerator->bounded(0, (ui->GroupMaze->height() / step)) * step));
      } while (ui->Player->pos() - QPoint(PlayerOffset, PlayerOffset) == ui->label_Finish->pos() - (ui->GroupMaze->pos() + QPoint(1, 1)));
      createMaze();
      balanceChange(markers * 5);
      markers = 0;
   }
   QMainWindow::keyPressEvent(event);
}
void MazeWindow::checkRank()
{
   QString tempRank = "";
   if (score >= 1000)
      tempRank = ":/image/Rank18.png";
   else if (score >= 800)
      tempRank = ":/image/Rank17.png";
   else if (score >= 700)
      tempRank = ":/image/Rank16.png";
   else if (score >= 600)
      tempRank = ":/image/Rank15.png";
   else if (score >= 500)
      tempRank = ":/image/Rank14.png";
   else if (score >= 400)
      tempRank = ":/image/Rank13.png";
   else if (score >= 340)
      tempRank = ":/image/Rank12.png";
   else if (score >= 250)
      tempRank = ":/image/Rank11.png";
   else if (score >= 190)
      tempRank = ":/image/Rank10.png";
   else if (score >= 150)
      tempRank = ":/image/Rank9.png";
   else if (score >= 120)
      tempRank = ":/image/Rank8.png";
   else if (score >= 90)
      tempRank = ":/image/Rank7.png";
   else if (score >= 65)
      tempRank = ":/image/Rank6.png";
   else if (score >= 40)
      tempRank = ":/image/Rank5.png";
   else if (score >= 30)
      tempRank = ":/image/Rank4.png";
   else if (score >= 20)
      tempRank = ":/image/Rank3.png";
   else if (score >= 10)
      tempRank = ":/image/Rank2.png";
   else if (score > 0)
      tempRank = ":/image/Rank1.png";
   ui->label_Rank->setPixmap(QPixmap(tempRank));
}
bool MazeWindow::balanceChange(double value)
{
   QString currentBalanceText = ui->label_Balance->text();
   currentBalanceText.chop(5);
   double newBalance = currentBalanceText.toDouble() + value;

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
         double newBalance = currentBalanceText1.toDouble() + value;
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
