#ifndef ITEMINFODIALOG_H
#define ITEMINFODIALOG_H
#include "MazeWindow.h"

#include <QDialog>

namespace Ui {
class ItemInfoDialog;
}

class ItemInfoDialog : public QDialog
{
   Q_OBJECT

public:
   explicit ItemInfoDialog(MazeWindow *parent, MazeWindow::item current_Item, int Item_Index = -1);
   int Item_Index = 0;
   bool sold = false;
   ~ItemInfoDialog();
   Ui::ItemInfoDialog *ui;
};

#endif // ITEMINFODIALOG_H
