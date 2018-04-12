
#include <QPainter>

#include "graphedge.h"
#include "graphnode.h"

#include <math.h>

static const double Pi = 3.14159265358979323846264338327950288419717;
static double TwoPi = 2.0 * Pi;

GraphEdge::GraphEdge(GraphNode *sourceNode, GraphNode *destNode, QString text, bool addArrow)
 : arrowSize(7)
{
 setAcceptedMouseButtons(0);
 source = sourceNode;
 dest = destNode;
 source->addEdge(this);
 dest->addEdge(this);
 adjust();
 this->text = text;
 this->addArrow = addArrow;
}

GraphNode *GraphEdge::sourceNode() const
{
 return source;
}

GraphNode *GraphEdge::destNode() const
{
 return dest;
}

void GraphEdge::adjust()
{
 if (!source || !dest)
     return;

 QLineF line(mapFromItem(source, 0, 0), mapFromItem(dest, 0, 0));
 qreal length = line.length();

 prepareGeometryChange();

 if (length > qreal(20.)) {
     QPointF edgeOffset((line.dx() * 10) / length, (line.dy() * 10) / length);
     sourcePoint = line.p1() + edgeOffset;
     destPoint = line.p2() - edgeOffset;
 } else {
     sourcePoint = destPoint = line.p1();
 }
}

QRectF GraphEdge::boundingRect() const
{
 if (!source || !dest)
     return QRectF();

 qreal penWidth = 1;
 qreal extra = (penWidth + arrowSize) / 2.0;

 return QRectF(sourcePoint, QSizeF(destPoint.x() - sourcePoint.x(),
                                   destPoint.y() - sourcePoint.y()))
     .normalized()
     .adjusted(-extra, -extra, extra+30, extra);
}

void GraphEdge::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *)
{
 if (!source || !dest)
     return;

 QLineF line(sourcePoint, destPoint);
 if(qFuzzyCompare(line.length(), qreal(0.)))
     return;

 // Draw the line itself
 painter->setPen(QPen(Qt::black, 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
 painter->drawLine(line);

 // Draw the arrows
 if(addArrow) {
     double angle = ::acos(line.dx() / line.length());
     if (line.dy() >= 0)
         angle = TwoPi - angle;

     /*
     QPointF sourceArrowP1 = sourcePoint + QPointF(sin(angle + Pi / 3) * arrowSize,
                                                   cos(angle + Pi / 3) * arrowSize);
     QPointF sourceArrowP2 = sourcePoint + QPointF(sin(angle + Pi - Pi / 3) * arrowSize,
                                                   cos(angle + Pi - Pi / 3) * arrowSize);
     */
     QPointF destArrowP1 = destPoint + QPointF(sin(angle - Pi / 3) * arrowSize,
                                               cos(angle - Pi / 3) * arrowSize);
     QPointF destArrowP2 = destPoint + QPointF(sin(angle - Pi + Pi / 3) * arrowSize,
                                               cos(angle - Pi + Pi / 3) * arrowSize);

     painter->setBrush(Qt::black);
     //painter->drawPolygon(QPolygonF() << line.p1() << sourceArrowP1 << sourceArrowP2);
     painter->drawPolygon(QPolygonF() << line.p2() << destArrowP1 << destArrowP2);
 }
 /** Add text to line **/
 if(text.isEmpty()) {
     return;
 }
 painter->setPen(QPen(Qt::black, 0));
 qreal x = (source->x() + dest->x())/2;
 qreal y = (source->y() + dest->y())/2;
 //painter->drawEllipse(x, y, 20, 20);
 QRect rect(x,y-12.5,25,25);

 QFont font("Times", 4, QFont::Normal);
 painter->setFont(font);
 while(painter->fontMetrics().width(text) > rect.width()) {
     //int newsize = painter->font().pointSize() - 1;
     //painter->setFont(QFont(painter->font().family(), newsize));
     rect.setWidth(rect.width()+1);
 }
 painter->drawText(rect, Qt::AlignCenter, text);
}
