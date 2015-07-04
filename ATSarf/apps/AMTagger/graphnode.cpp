#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>
#include <QPainter>
#include <QStyleOption>

#include "graphedge.h"
#include "graphnode.h"
#include "global.h"

GraphNode::GraphNode(QString text, QString fgcolor, QString bgcolor)
{
    setFlag(ItemIsMovable);
    setFlag(ItemSendsGeometryChanges);
    setCacheMode(DeviceCoordinateCache);
    setZValue(-1);
    this->text = text;
    this->fgcolor = fgcolor;
    this->bgcolor = bgcolor;
}

void GraphNode::addEdge(GraphEdge *edge)
{
    edgeList << edge;
    edge->adjust();
}

QList<GraphEdge *> GraphNode::edges() const
{
    return edgeList;
}

void GraphNode::calculateForces()
{
    if (!scene() || scene()->mouseGrabberItem() == this) {
        newPos = pos();
        return;
    }

    // Sum up all forces pushing this item away
    qreal xvel = 0;
    qreal yvel = 0;
    foreach (QGraphicsItem *item, scene()->items()) {
        GraphNode *node = qgraphicsitem_cast<GraphNode *>(item);
        if (!node)
            continue;

        QPointF vec = mapToItem(node, 0, 0);
        qreal dx = vec.x();
        qreal dy = vec.y();
        double l = 2.0 * (dx * dx + dy * dy);
        if (l > 0) {
            xvel += (dx * 150.0) / l;
            yvel += (dy * 150.0) / l;
        }
    }

    // Now subtract all forces pulling items together
    double weight = (edgeList.size() + 1) * 10;
    foreach (GraphEdge *edge, edgeList) {
        QPointF vec;
        if (edge->sourceNode() == this)
            vec = mapToItem(edge->destNode(), 0, 0);
        else
            vec = mapToItem(edge->sourceNode(), 0, 0);
        xvel -= vec.x() / weight;
        yvel -= vec.y() / weight;
    }

    if (qAbs(xvel) < 0.1 && qAbs(yvel) < 0.1)
        xvel = yvel = 0;

    QRectF sceneRect = scene()->sceneRect();
    newPos = pos() + QPointF(xvel, yvel);
    newPos.setX(qMin(qMax(newPos.x(), sceneRect.left() + 10), sceneRect.right() - 10));
    newPos.setY(qMin(qMax(newPos.y(), sceneRect.top() + 10), sceneRect.bottom() - 10));
}

bool GraphNode::posAdvance()
{
    if (newPos == pos())
        return false;

    setPos(newPos);
    return true;
}

QRectF GraphNode::boundingRect() const
{
    qreal adjust = 2;
    return QRectF( -40 - adjust, -20 - adjust,
                   83 + adjust, 43 + adjust);
}

QPainterPath GraphNode::shape() const
{
    QPainterPath path;
    path.addEllipse(-20, -10, 40, 20);
    return path;
}

void GraphNode::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *)
{
    /** Draw text rectangle **/
    QRect textrect(-10,-10,20,20);

    QFont font("Times", 7, QFont::Normal);
    painter->setFont(font);
    while(painter->fontMetrics().width(text) > textrect.width()) {
        //int newsize = painter->font().pointSize() - 1;
        //painter->setFont(QFont(painter->font().family(), newsize));
        //textrect.setX(textrect.x()-0.5);
        //textrect.setWidth(textrect.width()+1);
        textrect.setWidth(textrect.width()+1);
        textrect.setX((-1)*textrect.width()/2);
    }
    if(textrect.width() != 20) {
        textrect.setWidth(textrect.width()+3);
        textrect.setX((-1)*textrect.width()/2);
    }
    /** End **/

    painter->setPen(Qt::NoPen);
    painter->setBrush(Qt::darkGray);
    painter->drawEllipse(((-1)*textrect.width()/2)+3, -7, textrect.width(), 20);

    if(bgcolor.isEmpty()) {

        /*
        QRadialGradient gradient(-3, -3, 10);
        if (option->state & QStyle::State_Sunken) {
            gradient.setCenter(3, 3);
            gradient.setFocalPoint(3, 3);
            gradient.setColorAt(1, QColor(Qt::yellow).light(120));
            gradient.setColorAt(0, QColor(Qt::darkYellow).light(120));
        } else {
            gradient.setColorAt(0, Qt::yellow);
            gradient.setColorAt(1, Qt::darkYellow);
        }
        painter->setBrush(gradient);
        */
        painter->setBrush(Qt::yellow);
        painter->setPen(QPen(Qt::black, 0));
    }
    else {
        painter->setBrush(QColor(bgcolor));
        painter->setPen(QPen(QColor(fgcolor),0));
    }

    painter->drawEllipse((-1)*textrect.width()/2, -10, textrect.width(), 20);

    /** Draw the text **/
    if(textrect.width() != 20) {
        textrect.setWidth(textrect.width()-3);
        textrect.setX((-1)*textrect.width()/2);
    }
    painter->drawText(textrect, Qt::AlignCenter, text);
}

QVariant GraphNode::itemChange(GraphicsItemChange change, const QVariant &value)
{
    switch (change) {
    case ItemPositionHasChanged:
        foreach (GraphEdge *edge, edgeList)
            edge->adjust();
        break;
    default:
        break;
    };

    return QGraphicsItem::itemChange(change, value);
}

void GraphNode::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    update();
    QGraphicsItem::mousePressEvent(event);
}

void GraphNode::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    update();
    QGraphicsItem::mouseReleaseEvent(event);
}
