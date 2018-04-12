#include "crossreferenceview.h"
#include "global.h"
#include "graphedge.h"
#include "graphnode.h"
#include <QDockWidget>

CrossReferenceView::CrossReferenceView(QWidget *parent) :
        QMainWindow(parent) {

    graphics = new QGraphicsView(this);
    scene = new QGraphicsScene(this);

    scene->setItemIndexMethod(QGraphicsScene::NoIndex);
    scene->setSceneRect(-300, -300, 600, 600);

    graphics->setScene(scene);
    graphics->setCacheMode(QGraphicsView::CacheBackground);
    graphics->setViewportUpdateMode(QGraphicsView::BoundingRectViewportUpdate);
    graphics->setRenderHint(QPainter::Antialiasing);
    graphics->setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
    graphics->scale(qreal(2), qreal(2));

    setCentralWidget(graphics);
    setWindowTitle(tr("Cross-Reference Graph"));
    resize(680, 420);

    /** Create the entity-relation graph of graphviz
        We use this graph to create the ER graph
        and calculate the coordinates of each node for better visualization **/
    GVC_t* gvc = gvContext();
    //Agedge_t *edge = NULL;
    char* args[] = {
    const_cast<char *>("dot"),
    const_cast<char *>("-Tsvg"),    /* svg output */
    const_cast<char *>("-oabc.svg") /* output to file abc.svg */
    };
    gvParseArgs(gvc, sizeof(args)/sizeof(char*), args);
    Agraph_t* G = agopen(const_cast<char *>("matchGraph"), Agstrictdirected, 0);
    //Set graph attributes
    agsafeset(G, const_cast<char *>("overlap"), const_cast<char *>("prism"),const_cast<char *>(""));
    agsafeset(G, const_cast<char *>("splines"), const_cast<char *>("true"),const_cast<char *>("true"));
    agsafeset(G, const_cast<char *>("pad"), const_cast<char *>("0,2"),const_cast<char *>("0,2"));
    agsafeset(G, const_cast<char *>("dpi"), const_cast<char *>("96,0"),const_cast<char *>("96,0"));
    agsafeset(G, const_cast<char *>("nodesep"), const_cast<char *>("0,4"),const_cast<char *>("0,4"));
    agattr(G,AGNODE,const_cast<char *>("label"),const_cast<char *>(""));
    agattr(G,AGNODE,const_cast<char *>("fixedsize"), const_cast<char *>("true"));
    agattr(G,AGNODE,const_cast<char *>("regular"), const_cast<char *>("true"));
    int id=0;

    QHash<QString, Agnode_t*> rmnHash;
    QSet<Triplet<Agnode_t*,Agnode_t*,QString>*> nodeedgeSet;
    for(int i=0; i< _atagger->simulationVector.count(); i++) {
        MERFTag* merftag = (MERFTag*)_atagger->simulationVector[i];
        for(int k=0; k<merftag->relationMatchVector.count(); k++) {
            RelationM* relMatch = merftag->relationMatchVector[k];
            Agnode_t* node1;
            QString e1ID = relMatch->entity1->msf->name;
            e1ID.append(QString::number(relMatch->entity1->getPOS()));
            if(rmnHash.contains(e1ID)) {
                node1 = rmnHash.value(e1ID);
            }
            else {
                stringstream strs;
                strs << id;
                string temp_str = strs.str();
                char* nodeID = strdup(temp_str.c_str());
                node1 = agnode(G,nodeID, 1);
                //agset(reG,const_cast<char *>("root"),nodeID);
                id = id+1;
                char * writable = strdup(relMatch->e1Label.toStdString().c_str());
                agset(node1,const_cast<char *>("label"),writable);
                rmnHash.insert(e1ID,node1);
            }

            Agnode_t* node2;
            QString e2ID = relMatch->entity2->msf->name;
            e2ID.append(QString::number(relMatch->entity2->getPOS()));
            if(rmnHash.contains(e2ID)) {
                node2 = rmnHash.value(e2ID);
            }
            else {
                stringstream strs;
                strs << id;
                string temp_str = strs.str();
                char* nodeID = strdup(temp_str.c_str());
                node2 = agnode(G,nodeID, 1);
                //agset(reG,const_cast<char *>("root"),nodeID);
                id = id+1;
                char * writable = strdup(relMatch->e2Label.toStdString().c_str());
                agset(node2,const_cast<char *>("label"),writable);
                rmnHash.insert(e2ID,node2);
            }

            //edge = agedge(G, node1, node2, 0, 1);
            agedge(G, node1, node2, 0, 1);
            Triplet<Agnode_t*,Agnode_t*,QString>* triplet = new Triplet<Agnode_t*,Agnode_t*,QString>(node1,node2,relMatch->edgeLabel);
            nodeedgeSet.insert(triplet);
            //edgesMap.insert(node1,node2);
        }
    }

    /** Add the cross reference relations **/

    for(int i=0; i<_atagger->crossRelationVector.count(); i++) {
        RelationM* relMatch = _atagger->crossRelationVector[i];
        Agnode_t* node1;
        QString e1ID = relMatch->entity1->msf->name;
        e1ID.append(QString::number(relMatch->entity1->getPOS()));
        if(rmnHash.contains(e1ID)) {
            node1 = rmnHash.value(e1ID);
        }
        else {
            stringstream strs;
            strs << id;
            string temp_str = strs.str();
            char* nodeID = strdup(temp_str.c_str());
            node1 = agnode(G,nodeID, 1);
            //agset(reG,const_cast<char *>("root"),nodeID);
            id = id+1;
            char * writable = strdup(relMatch->e1Label.toStdString().c_str());
            agset(node1,const_cast<char *>("label"),writable);
            rmnHash.insert(e1ID,node1);
        }

        Agnode_t* node2;
        QString e2ID = relMatch->entity2->msf->name;
        e2ID.append(QString::number(relMatch->entity2->getPOS()));
        if(rmnHash.contains(e2ID)) {
            node2 = rmnHash.value(e2ID);
        }
        else {
            stringstream strs;
            strs << id;
            string temp_str = strs.str();
            char* nodeID = strdup(temp_str.c_str());
            node2 = agnode(G,nodeID, 1);
            //agset(reG,const_cast<char *>("root"),nodeID);
            id = id+1;
            char * writable = strdup(relMatch->e2Label.toStdString().c_str());
            agset(node2,const_cast<char *>("label"),writable);
            rmnHash.insert(e2ID,node2);
        }

        //edge = agedge(G, node1, node2, 0, 1);
        agedge(G, node1, node2, 0, 1);
        Triplet<Agnode_t*,Agnode_t*,QString>* triplet = new Triplet<Agnode_t*,Agnode_t*,QString>(node1,node2,relMatch->edgeLabel);
        nodeedgeSet.insert(triplet);
        //edgesMap.insert(node1,node2);
    }

    /** Set layout of the graph and get coordinates **/
    /* Compute a layout using layout engine from command line args */
    gvLayoutJobs(gvc, G);
    //gvRenderJobs(gvc, G);

    /** Set scene rectangle **/
    int left = GD_bb(G).LL.x;
    int top = GD_bb(G).LL.y;
    int width = GD_bb(G).UR.x;
    int height = GD_bb(G).UR.y;
    QRectF rerect(left,top,width,height);
    scene->setSceneRect(rerect);

    /** Get coordinates of nodes and draw nodes in scene **/
    QMap<Agnode_t*,GraphNode *> renodes;
    QSetIterator<Triplet<Agnode_t *,Agnode_t *,QString>*> reiterator(nodeedgeSet);
    while (reiterator.hasNext()) {
        Triplet<Agnode_t*,Agnode_t*,QString>* triplet = reiterator.next();
        /// Use this string to know if an edge label should be added
        QString edgeLabel(triplet->third);

        Agnode_t* tmpNode1 = triplet->first;
        char* label1 = agget(tmpNode1,const_cast<char *>("label"));
        qreal nodeX1 = ND_coord(tmpNode1).x;
        qreal nodeY1 = (GD_bb(G).UR.y - ND_coord(tmpNode1).y);
        GraphNode *node1;
        if(!(renodes.contains(tmpNode1))) {
            QString stringlabel1(label1);
            node1 = new GraphNode(stringlabel1,"","");
            scene->addItem(node1);
            node1->setPos(nodeX1,nodeY1);
            renodes.insert(tmpNode1,node1);
        }
        else {
            node1 = renodes.value(tmpNode1);
        }

        Agnode_t* tmpNode2 = triplet->second;
        if(tmpNode2 == NULL) {
            continue;
        }
        char* label2 = agget(tmpNode2,const_cast<char *>("label"));
        qreal nodeX2 = ND_coord(tmpNode2).x;
        qreal nodeY2 = (GD_bb(G).UR.y - ND_coord(tmpNode2).y);
        GraphNode *node2;
        if(!(renodes.contains(tmpNode2))) {
            QString stringlabel2(label2);
            node2 = new GraphNode(stringlabel2,"","");
            scene->addItem(node2);
            node2->setPos(nodeX2,nodeY2);
            renodes.insert(tmpNode2,node2);
            if(edgeLabel.isEmpty()) {
                scene->addItem(new GraphEdge(node2,node1,"",false));
            }
            else {
                scene->addItem(new GraphEdge(node2,node1,edgeLabel,false));
                edgeLabel.clear();
            }
        }
        else {
            node2 = renodes.value(tmpNode2);
            if(edgeLabel.isEmpty()) {
                scene->addItem(new GraphEdge(node2,node1,"",false));
            }
            else {
                scene->addItem(new GraphEdge(node2,node1,edgeLabel,false));
                edgeLabel.clear();
            }
        }
        delete triplet;
    }
    nodeedgeSet.clear();

    /* Free layout data */
    gvFreeLayout(gvc, G);
    /* Free graph structures */
    agclose(G);
    /* close output file and free context */
    gvFreeContext(gvc);
}
