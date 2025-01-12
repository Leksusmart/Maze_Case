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
   ~ItemInfoDialog();
   bool sold = false;

private:
   Ui::ItemInfoDialog *ui;
   int Item_Index = 0;
};

#endif // ITEMINFODIALOG_H
