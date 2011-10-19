#ifndef GENEMAINWINDOW_H
#define GENEMAINWINDOW_H

#include <QMainWindow>

#include <QAbstractItemModel>
#include <QModelIndex>
#include <QVariant>

class QTreeView;
class QLabel;
class QScrollArea;
class GeneTree;
class GeneItemModel;

class GeneMainWindow : public QMainWindow
{
	Q_OBJECT

	public:
		GeneMainWindow();
		virtual ~GeneMainWindow();
		void setText(const QString & );
		void display(GeneTree * tree,QString imageFilename,bool deleteLater=false);

	private slots:
		void newFile();
		void openFile();
		void save();

		void about();
		void resultChanged (const QString &customer);
		void behaviorChanged(const QString &paragraph);



	private:
		void createActions();
		void createMenus();
		void createToolBars();
		void createStatusBar();
		void createDockWindows();

		void openFileEx();
		void clean() ;

		GeneTree * tree;
		bool deleteTreeWhenFinished;

		QTreeView *resultTree;

		QMenu *fileMenu;
		QMenu *editMenu;
		QMenu *viewMenu;
		QMenu *checkMenu;
		QMenu *helpMenu;

		QToolBar *fileToolBar;
		QToolBar *editToolBar;
		QToolBar *checkToolBar;

		QAction *newFileAct;
		QAction *openFileAct;
		QAction *saveAct;
		QAction *aboutAct;
		QAction *quitAct;

		QLabel * graphDisplay;
		QScrollArea * graphArea;

		GeneItemModel * model;

};

#endif // GENEMAINWINDOW_H
