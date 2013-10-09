#ifndef GRAPHNODE_H
#define GRAPHNODE_H

#include <QGraphicsItem>
#include <QList>

class GraphEdge;
class GraphWidget;
class QGraphicsSceneMouseEvent;

class GraphNode : public QGraphicsItem
{
public:
    GraphNode(QString text);

     void addEdge(GraphEdge *edge);
     QList<GraphEdge *> edges() const;

     enum { Type = UserType + 1 };
     int type() const { return Type; }

     void calculateForces();
     bool advance();

     QRectF boundingRect() const;
     QPainterPath shape() const;
     void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

protected:
     QVariant itemChange(GraphicsItemChange change, const QVariant &value);

     void mousePressEvent(QGraphicsSceneMouseEvent *event);
     void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);

private:
     QList<GraphEdge *> edgeList;
     QPointF newPos;
     QString text;
};

#endif
