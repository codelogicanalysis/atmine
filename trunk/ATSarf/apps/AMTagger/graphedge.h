#ifndef GRAPHEDGE_H
#define GRAPHEDGE_H

#include <QGraphicsItem>

class GraphNode;

class GraphEdge : public QGraphicsItem
{
public:
 GraphEdge(GraphNode *sourceNode, GraphNode *destNode, QString text="");

 GraphNode *sourceNode() const;
 GraphNode *destNode() const;

 void adjust();

 enum { Type = UserType + 2 };
 int type() const { return Type; }

protected:
 QRectF boundingRect() const;
 void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

private:
 GraphNode *source, *dest;

 QPointF sourcePoint;
 QPointF destPoint;
 qreal arrowSize;
 QString text;
};

#endif
