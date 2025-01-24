#include "ItemInfoDialog.h"
#include "MazeWindow.h"
#include "ui_ItemInfoDialog.h"
#include "ui_MazeWindow.h"

#include <QStyle>

#include <QDesktopServices>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>

ItemInfoDialog::ItemInfoDialog(MazeWindow *parent, MazeWindow::item current_Item, int Item_Index, bool isDev)
   : QDialog(parent)
   , parent(parent)
   , ui(new Ui::ItemInfoDialog)
   , Item_Index(Item_Index)
   , networkManager(new QNetworkAccessManager(this))
   , isDev(isDev)
{
   ui->setupUi(this);
   this->setWindowTitle("О предмете");
   this->setWindowIcon(style()->standardIcon(QStyle::SP_MessageBoxInformation));

   connect(ui->toolButton, &QToolButton::clicked, this, [this, current_Item, parent]() {
      auto it = std::find_if(parent->itemDetails.begin(), parent->itemDetails.end(), [this, current_Item](MazeWindow::ItemDetail &Item) { return Item.source == current_Item.photo; });
      if ((it->steamID[0] == it->steamID[1] && it->steamID[1] == it->steamID[2] && it->steamID[2] == it->steamID[3] && it->steamID[3] == it->steamID[4]) || current_Item.isCase) {
         QDesktopServices::openUrl(QUrl("https://steamcommunity.com/market/listings/730/" + it->nameEng));
         qDebug() << "Opened in browser:" << "https://steamcommunity.com/market/listings/730/" + it->nameEng;
      } else {
         QString FloatStr = "";
         if (current_Item.Float >= 0.45)
            FloatStr = "(Battle-Scarred)";
         else if (current_Item.Float >= 0.37)
            FloatStr = "(Well-Worn)";
         else if (current_Item.Float >= 0.15)
            FloatStr = "(Field-Tested)";
         else if (current_Item.Float >= 0.07)
            FloatStr = "(Minimal Wear)";
         else
            FloatStr = "(Factory New)";
         QDesktopServices::openUrl(QUrl("https://steamcommunity.com/market/listings/730/" + it->nameEng + " " + FloatStr));
         qDebug() << "Opened in browser:" << "https://steamcommunity.com/market/listings/730/" + it->nameEng + " " + FloatStr;
      }
      this->reject();
   });
   connect(ui->pushButton_Sold, &QPushButton::clicked, this, [this, parent, Item_Index, &current_Item, isDev]() {
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
      parent->getInventory(Item_Index, isDev);
      this->reject();
   });
   ui->pushButton_Open->hide();
   if (current_Item.isCase) {
      ui->label_Float->hide();
      ui->labelStatic_Float->hide();
      ui->label_Float_Name->hide();
      ui->pushButton_Open->show();
      connect(ui->pushButton_Open, &QPushButton::clicked, this, [this, isDev, parent, Item_Index]() {
         if (parent->balanceChange(-265)) {
            parent->getInventory(Item_Index, isDev);
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
   QString FloatStr = "";
   if (current_Item.Float >= 0.45) {
      FloatStr = "(Battle-Scarred)";
      ui->label_Float_Name->setText("Закаленное в боях");
   } else if (current_Item.Float >= 0.37) {
      FloatStr = "(Well-Worn)";
      ui->label_Float_Name->setText("Поношенное");
   } else if (current_Item.Float >= 0.15) {
      FloatStr = "(Field-Tested)";
      ui->label_Float_Name->setText("После полевых испытаний");
   } else if (current_Item.Float >= 0.07) {
      FloatStr = "(Minimal Wear)";
      ui->label_Float_Name->setText("Немного поношенное");
   } else if (current_Item.Float >= 0.0) {
      FloatStr = "(Factory New)";
      ui->label_Float_Name->setText("Прямо с завода");
   }

   ui->label_Photo->setPixmap(QPixmap(current_Item.photo).scaled(ui->label_Photo->size(), Qt::KeepAspectRatioByExpanding, Qt::FastTransformation));
   ui->label_Photo->raise();
   ui->label_Photo->setScaledContents(true);
}
void ItemInfoDialog::updatePrice(MazeWindow::item item)
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

   ui->labelStatic_Cost->setText("Уточнение...");
   ui->label_Cost->setText("");

   QString url = QString("https://steamcommunity.com/market/itemordershistogram?&language=english&currency=5&item_nameid=%1").arg(ITEM_ID);
   QNetworkReply *reply = networkManager->get(QNetworkRequest(QUrl(url)));

   connect(reply, &QNetworkReply::finished, [this, reply, index, it]() {
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

            qDebug() << QString("Обновлена стоимость: " + it->name + " -> %1").arg(cost, 0, 'f', 2);

            ui->label_Cost->setText(QString("%1 руб.").arg(cost, 0, 'f', 2));
            it->cost[index] = QString("%1").arg(cost, 0, 'f', 2);

            ui->labelStatic_Cost->setText("Это стоит:");
            if (it->source == parent->itemDetails[1 - 1].source)
               parent->ui->label_Item1_Name->setText(QString("%1 %2 руб.").arg(parent->itemDetails[1 - 1].name, parent->itemDetails[1 - 1].cost[0]));
            else if (it->source == parent->itemDetails[2 - 1].source)
               parent->ui->label_Item2_Name->setText(QString("%1 %2 руб.").arg(parent->itemDetails[2 - 1].name, parent->itemDetails[2 - 1].cost[0]));
            else if (it->source == parent->itemDetails[3 - 1].source)
               parent->ui->label_Item3_Name->setText(QString("%1 %2 руб.").arg(parent->itemDetails[3 - 1].name, parent->itemDetails[3 - 1].cost[0]));
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
