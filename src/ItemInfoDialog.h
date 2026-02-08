#ifndef ITEMINFODIALOG_H
#define ITEMINFODIALOG_H
#include "MazeWindow.h"

#include <QDesktopServices>
#include <QDialog>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QStyle>

namespace Ui { class ItemInfoDialog; }

class ItemInfoDialog : public QDialog
{
public:
   explicit ItemInfoDialog(MazeWindow *parent, MazeWindow::item current_Item, int Item_Index = -1, bool isDev = false);
   ~ItemInfoDialog();
   bool sold = false;
private slots:
   void updatePrice(MazeWindow::item item);

private:
   MazeWindow *parent;
   Ui::ItemInfoDialog *ui;
   int Item_Index = 0;
   bool isDev = false;
   QNetworkAccessManager *networkManager;
private slots:
   void closeEvent(QCloseEvent *event) override;
};

#endif // ITEMINFODIALOG_H
