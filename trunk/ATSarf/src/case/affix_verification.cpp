#include <QTextStream>
#include <QFile>
#include "common.h"
#include <QQueue>
#include "tree.h"
#include "test.h"
#include "database_info_block.h"
#include "inflections.h"
#include "affixTreeTraversal.h"
#include <QString>
#include <QMainWindow>
#include <QScrollArea>
#include <QLabel>
#include <QDir>

#define SHOW_CATEGORY_LABEL
#define SHOW_CATEGORY
#define SHOW_RAW_DATA
typedef QMap<node *,int> AffixGraphMap;

inline int getGraphID(AffixGraphMap & map,int & last_id, node * n) {
	int curr_id;
	AffixGraphMap::iterator it=map.find(n);
	if (it==map.end()) {
		curr_id=++last_id;
		map.insert(n,curr_id);
	}
	else
		curr_id=it.value();
	return curr_id;
}
void drawAffixGraph(item_types type) {
	int last_id=0;
	AffixGraphMap map;
	map.clear();
	QString t=interpret_type(type);
	QQueue<node *> queue;
	node * base=(type==PREFIX?database_info.Prefix_Tree->getFirstNode():database_info.Suffix_Tree->getFirstNode());
	QFile file(QString("%1.dot").arg(t));
	file.remove();
	if (!file.open(QIODevice::ReadWrite))
	{
		out<<"Error openning file\n";
		return;
	}

	QTextStream d_out(&file);
	d_out.setCodec("utf-8");
	d_out<<"digraph tree {\n";
	queue.enqueue(base);

	while (!queue.isEmpty())
	{
		node & n=*(queue.dequeue());
		if (n.isLetterNode()) {
			letter_node & l= (letter_node &)n;
			QChar letter=l.getLetter();
			d_out<<"n"<<getGraphID(map,last_id,&n)<<" [label=\""<<(letter!='\0'?letter:'-')<<"\", shape=circle];\n";
		}else {
			result_node & r= (result_node &)n;
		#ifdef SHOW_CATEGORY
			d_out<<"n"<<getGraphID(map,last_id,&n)<<" [label=\""
				#ifdef SHOW_CATEGORY_LABEL
					<<database_info.comp_rules->getCategoryName(r.get_resulting_category_id())
				#endif
					<<"\", shape=rectangle];\n";
		#endif
		#ifdef SHOW_RAW_DATA
			int size=r.raw_datas.size();
			for (int i=0;i<size;i++) {
				d_out<<"r"<<getGraphID(map,last_id,&n)<<"_"<<i<<" [label=\""<<r.raw_datas[i].getActual()<<"\", shape=oval, color=red];\n";
			#ifdef SHOW_CATEGORY
				d_out<<"n"<<getGraphID(map,last_id,&n)
			#else
				d_out<<"n"<<getGraphID(map,last_id,(n.isLetterNode()?&n:(n.getPrevious())))
			#endif
						<<"->"<<"r"<<getGraphID(map,last_id,&n)<<"_"<<i<<";\n";
			}
		#endif
		}
		QVector<letter_node *> l_nodes=n.getLetterChildren();
		QList<result_node *> r_nodes =*n.getResultChildren();
		int size=l_nodes.size();
		for (int i=0; i<size;i++) {
			node * nc=l_nodes[i];
			if (nc!=NULL) {
				queue.enqueue(nc);
			#if defined(SHOW_CATEGORY)
				d_out<<"n"<< getGraphID(map,last_id,&n)<<"->"<<"n"<<getGraphID(map,last_id,nc)<<";\n";
			#elif defined(SHOW_RAW_DATA)
				if (n.isLetterNode())
					d_out<<"n"<< getGraphID(map,last_id,&n)<<"->"<<"n"<<getGraphID(map,last_id,nc)<<";\n";
				else {
					result_node * rn=(result_node*)&n;
					QList<QString> l=rn->raw_datas;
					for (int i=0;i<l.size();i++) {
						d_out<<"r"<<getGraphID(map,last_id,rn)<<"_"<<i<<"->"<<"n"<<getGraphID(map,last_id,nc)<<";\n";
					}
				}
			#else
				d_out<<"n"<< getGraphID(map,last_id,(n.isLetterNode()?&n:(n.getPrevious())))<<"->"<<"n"<<getGraphID(map,last_id,nc)<<";\n";
			#endif

			}
		}

		size=r_nodes.size();
		for (int i=0; i<size;i++) {
			node * nc=r_nodes[i];
			if (nc!=NULL) {
				queue.enqueue(nc);
			#ifdef SHOW_CATEGORY
				d_out<<"n"<<getGraphID(map,last_id,&n)<<"->"<<"n"<<getGraphID(map,last_id,nc)<<";\n";
			#endif
			}
		}

	}
	d_out<<"}\n";
	file.close();
	try{
#ifdef DISPLAY
		system(QString("dot -Tsvg %1.dot -o %1.svg").arg(t).toStdString().data());
		QMainWindow * mw =new QMainWindow(NULL);
		mw->setWindowTitle(QString("%1 Tree").arg(t));
		QScrollArea * sa=new QScrollArea(mw);
		mw->setCentralWidget(sa);
		QLabel *pic=new QLabel(sa);
		pic->setPixmap(QPixmap(QString("./%1.svg").arg(t)));
		sa->setWidget(pic);
		mw->show();
#else
		system(QString("kgraphviewer ./%1.dot -caption \"%1.dot\" &").arg(t).arg(QDir::currentPath()).toStdString().data());
#endif
	}
	catch(...)
	{}
}

class ListAllAffixes: public AffixTraversal {
private:
	QFile * file;
	QTextStream * dout;
	#define d_out *(dout)

	void visit(node *,QString affix, QString raw_data, long category_id, QString description, QString POS) {
		QString category=database_info.comp_rules->getCategoryName(category_id);
		d_out<<affix<<"\t"<<raw_data<<"\t"<<category<<"\t"<<description<<"\t"<<POS<<"\t\n";
	}
public:
	ListAllAffixes(item_types type):AffixTraversal(type,true) {
		QString t=interpret_type(type);
		file=new QFile(QString("%1.list").arg(t));
		file->remove();
		if (!file->open(QIODevice::ReadWrite)) {
			out<<"Error openning file\n";
			return;
		}
		dout=new QTextStream(file);
		dout->setCodec("utf-8");
	}
	~ListAllAffixes() {
		file->close();
		QString t=interpret_type(type);
		system(QString("sort %1.list >%1_sorted.list ").arg(t).toStdString().data());
		delete file;
		delete dout;
	}
};

void listAllAffixes(item_types type) {
	ListAllAffixes l(type);
	l();
}
