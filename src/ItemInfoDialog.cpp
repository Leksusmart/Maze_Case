#include "ItemInfoDialog.h"
#include "MazeWindow.h"
#include "ui_ItemInfoDialog.h"

#include <QStyle>

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
ItemInfoDialog::ItemInfoDialog(MazeWindow *parent, MazeWindow::item &current_Item, int Item_Index)
   : QDialog(parent)
   , parent(parent)
   , ui(new Ui::ItemInfoDialog)
   , Item_Index(Item_Index)
   , networkManager(new QNetworkAccessManager(this))
{
   ui->setupUi(this);
   this->setWindowTitle("О предмете");
   this->setWindowIcon(style()->standardIcon(QStyle::SP_MessageBoxInformation));

   connect(ui->pushButton_Sold, &QPushButton::clicked, this, [this, parent, Item_Index, &current_Item]() {
      sold = true;
      if (current_Item.cost == 0) {
         auto it = std::find_if(parent->itemDetails.begin(), parent->itemDetails.end(), [this, current_Item](MazeWindow::ItemDetail &Item) { return Item.source == current_Item.photo; });
         int index = 0;
         if (current_Item.Float >= 0.45)
            index = 4;
         else if (current_Item.Float >= 0.37)
            index = 3;
         else if (current_Item.Float >= 0.15)
            index = 2;
         else if (current_Item.Float >= 0.07)
            index = 1;
         else
            index = 0;
         parent->balanceChange(it->cost[index].toDouble());
      } else
         parent->balanceChange(current_Item.cost);
      parent->getInventory(Item_Index);
      this->reject();
   });
   ui->pushButton_Open->hide();
   if (current_Item.isCase) {
      ui->label_Float->hide();
      ui->labelStatic_Float->hide();
      ui->label_Float_Name->hide();
      ui->pushButton_Open->show();
      connect(ui->pushButton_Open, &QPushButton::clicked, this, [this, parent, Item_Index]() {
         if (parent->balanceChange(-265)) {
            parent->getInventory(Item_Index);
            this->accept();
         }
      });
      updatePrice(current_Item);
   } else {
      updatePrice(current_Item);
      this->accept();
   }

   ui->label_Name->setText(current_Item.name);
   ui->label_Float->setText(QString::number(current_Item.Float, 'f', 9));
   if (current_Item.Float >= 0.45)
      ui->label_Float_Name->setText("Закаленное в боях");
   else if (current_Item.Float >= 0.37)
      ui->label_Float_Name->setText("Поношенное");
   else if (current_Item.Float >= 0.15)
      ui->label_Float_Name->setText("После полевых испытаний");
   else if (current_Item.Float >= 0.07)
      ui->label_Float_Name->setText("Немного поношенное");
   else
      ui->label_Float_Name->setText("Прямо с завода");

   ui->label_Photo->setPixmap(QPixmap(current_Item.photo).scaled(ui->label_Photo->size(), Qt::KeepAspectRatioByExpanding, Qt::FastTransformation));
   ui->label_Photo->raise();
   ui->label_Photo->setScaledContents(true);
}
void ItemInfoDialog::updatePrice(MazeWindow::item &item)
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
   for (int i = 0; i < parent->itemDetails.size(); i++) {
      if (parent->itemDetails[i].name == item.name) {
         ITEM_ID = parent->itemDetails[i].steamID[index];
         break;
      }
   }
   if (ITEM_ID == "")
      return;

   // Ищем элемент в списке
   auto it = std::find_if(parent->itemDetails.begin(), parent->itemDetails.end(), [this, item](MazeWindow::ItemDetail &Item) { return Item.source == item.photo; });
   if (it->upToDate[index] == true) {
      item.cost = it->cost[index].toDouble();
      ui->label_Cost->setText(it->cost[index] + " руб.");
      return;
   }
   ui->labelStatic_Cost->setText("Уточнение...");
   ui->label_Cost->setText("");

   QString url = QString("https://steamcommunity.com/market/itemordershistogram?&language=english&currency=5&item_nameid=%1").arg(ITEM_ID);
   qDebug() << url;
   QNetworkReply *reply = networkManager->get(QNetworkRequest(QUrl(url)));

   connect(reply, &QNetworkReply::finished, [this, reply, &item, index, it]() {
      if (reply != nullptr) {
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

            ui->label_Cost->setText(QString("%1 руб.").arg(cost, 0, 'f', 2));
            item.cost = cost;
            it->cost[index] = QString("%1").arg(cost, 0, 'f', 2);
            it->upToDate[index] = true;

            ui->labelStatic_Cost->setText("Это стоит:");
         } else {
            ui->labelStatic_Cost->setText("Это стоит:");
            ui->label_Cost->setText("~" + it->cost[index] + " руб.");
            qDebug() << "Ошибка при получении данных:" << reply->errorString();
         }
         reply->deleteLater();
      }
   });
}
void ItemInfoDialog::closeEvent(QCloseEvent *event)
{
   this->reject();
   event->accept();
}
ItemInfoDialog::~ItemInfoDialog()
{
   networkManager->deleteLater();
   delete ui;
}
