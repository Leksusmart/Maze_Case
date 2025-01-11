#include "ItemInfoDialog.h"
#include <QStyle>
#include "MazeWindow.h"
#include "ui_ItemInfoDialog.h"
ItemInfoDialog::ItemInfoDialog(MazeWindow *parent, MazeWindow::item current_Item, int Item_Index)
   : QDialog(parent)
   , ui(new Ui::ItemInfoDialog)
   , Item_Index(Item_Index)
{
   ui->setupUi(this);
   this->setWindowTitle("О предмете");
   this->setWindowIcon(style()->standardIcon(QStyle::SP_MessageBoxInformation));
   connect(ui->pushButton_Sold, &QPushButton::clicked, this, [this, parent, Item_Index, current_Item]() {
      sold = true;
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
      ui->label_Cost->setText(QString::number(current_Item.cost) + " руб.");
   } else {
      ui->label_Cost->setText("~" + QString::number(current_Item.cost) + " руб.");
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

   ui->label_Photo->setPixmap(QPixmap(current_Item.photo).scaled(ui->label_Photo->size(), Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation));
   ui->label_Photo->setScaledContents(true);
}

ItemInfoDialog::~ItemInfoDialog()
{
   delete ui;
}
