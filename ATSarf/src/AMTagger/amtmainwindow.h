#ifndef AMTMAINWINDOW_H
#define AMTMAINWINDOW_H

#include <QMainWindow>

namespace Ui {
class AMTMainWindow;
}

class AMTMainWindow : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit AMTMainWindow(QWidget *parent = 0);
    ~AMTMainWindow();
    
private:
    Ui::AMTMainWindow *ui;
};

#endif // AMTMAINWINDOW_H
