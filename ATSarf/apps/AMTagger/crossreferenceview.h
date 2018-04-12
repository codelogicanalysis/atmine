#ifndef CROSSREFERENCEVIEW_H
#define CROSSREFERENCEVIEW_H

#include <QMainWindow>
#include <QGraphicsView>
#include <QGraphicsScene>

class CrossReferenceView : public QMainWindow
{
    Q_OBJECT
public:
    explicit CrossReferenceView(QWidget *parent = 0);
private:
    QTabWidget* tab;
    QGraphicsScene *scene;
    QGraphicsView *graphics;
};
#endif // CROSSREFERENCEVIEW_H
