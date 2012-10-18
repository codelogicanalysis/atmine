#include <QtGui>

#include "geneMainwindow.h"
#include "genealogyItem.h"


GeneMainWindow::GeneMainWindow() :
	tree(NULL), model(NULL)
{

	//setCentralWidget(codeEdit);

	createActions();

	createMenus();
	//createToolBars();
	createStatusBar();
	createDockWindows();

	setWindowTitle(tr("Graph Display"));

//    newFile();
	setUnifiedTitleAndToolBarOnMac(true);
	deleteTreeWhenFinished=false;
	tree=NULL;
}

GeneMainWindow::~GeneMainWindow()
{
	if (model!=NULL)
		delete model;
	if (deleteTreeWhenFinished && tree!=NULL)
		delete tree;
}

void GeneMainWindow::setText(const QString & ) {

}

void GeneMainWindow::newFile()
{

}


void
GeneMainWindow::openFileEx()
{
}


void
GeneMainWindow::openFile()
{
}



void GeneMainWindow::save()
{

}


void GeneMainWindow::resultChanged (const QString &)
{
}


void GeneMainWindow::behaviorChanged(const QString &)
{
}


void GeneMainWindow::about()
{
   /*QMessageBox::about(this, tr("About SpecCheck!"),
			tr("The <b>SpecCheck!</b> tool demonstrates a methodology "

			   "to build correct software incrementally."
			   " This is brought to you by the Software Analysis Research Lab (SARLab) at the American University of Beirut."));*/
}

void GeneMainWindow::createActions()
{
	newFileAct = new QAction(QIcon("./images/new.png"), tr("&New File"), this);
	newFileAct->setShortcuts(QKeySequence::New);
	newFileAct->setStatusTip(tr("Create a new file"));
	connect(newFileAct, SIGNAL(triggered()), this, SLOT(newFile()));

	openFileAct = new QAction(QIcon("./images/open.png"), tr("&Open File"), this);
	openFileAct->setShortcuts(QKeySequence::Open);
	openFileAct->setStatusTip(tr("Open a file"));
	connect(openFileAct, SIGNAL(triggered()), this, SLOT(openFile()));

	saveAct = new QAction(QIcon("./images/save.png"), tr("&Save..."), this);
	saveAct->setShortcuts(QKeySequence::Save);
	saveAct->setStatusTip(tr("Save file"));
	connect(saveAct, SIGNAL(triggered()), this, SLOT(save()));

	quitAct = new QAction(tr("&Quit"), this);
	quitAct->setShortcuts(QKeySequence::Quit);
	quitAct->setStatusTip(tr("Quit the application"));
	connect(quitAct, SIGNAL(triggered()), this, SLOT(close()));

	aboutAct = new QAction(tr("&About"), this);
	aboutAct->setStatusTip(tr("Show the application's About box"));
	connect(aboutAct, SIGNAL(triggered()), this, SLOT(about()));
}

void GeneMainWindow::createMenus()
{
	fileMenu = menuBar()->addMenu(tr("&File"));
	fileMenu->addAction(newFileAct);
	fileMenu->addAction(openFileAct);
	fileMenu->addAction(saveAct);
	fileMenu->addSeparator();
	fileMenu->addAction(quitAct);

	editMenu = menuBar()->addMenu(tr("&Edit"));
	//editMenu->addAction(undoAct);

	viewMenu = menuBar()->addMenu(tr("&View"));
	menuBar()->addSeparator();

	helpMenu = menuBar()->addMenu(tr("&Help"));
	helpMenu->addAction(aboutAct);
}

void GeneMainWindow::createToolBars()
{
	fileToolBar = addToolBar(tr("File"));
	fileToolBar->addAction(newFileAct);
	fileToolBar->addAction(openFileAct);
	fileToolBar->addAction(saveAct);

	editToolBar = addToolBar(tr("Edit"));
	//editToolBar->addAction(undoAct);

}


void GeneMainWindow::createStatusBar()
{
	statusBar()->showMessage(tr("Ready"));
}



void GeneMainWindow::createDockWindows()
{
	QDockWidget *dock = new QDockWidget(tr("Tree View"), this);
	dock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
	resultTree = new QTreeView (dock);
//    specTree->setWordWrap(true);
	resultTree->setSelectionBehavior (QAbstractItemView::SelectRows);
	resultTree->setSelectionMode (QAbstractItemView::SingleSelection);
	resultTree->setAlternatingRowColors (true);
	dock->setWidget(resultTree);
	addDockWidget(Qt::RightDockWidgetArea, dock);
//    addDockWidget(Qt::RightDockWidgetArea, dock);
	viewMenu->addAction(dock->toggleViewAction());

	dock = new QDockWidget(tr("Graph View"), this);
	dock->setAllowedAreas(Qt::AllDockWidgetAreas);
	graphDisplay = new QLabel(dock);
	graphArea=new QScrollArea(dock);
	dock->setWidget(graphArea);
	graphArea->setWidget(graphDisplay);
	graphArea->setWidgetResizable(true);
	graphArea->setAlignment(Qt::AlignCenter);
	//addDockWidget(Qt::BottomDockWidgetArea, dock);
	addDockWidget(Qt::RightDockWidgetArea, dock);
	viewMenu->addAction(dock->toggleViewAction());
	model=NULL;
}

void GeneMainWindow::display(GeneTree * tree,QString imageFileName,bool deleteLater) {
	if (model!=NULL)
		delete model;
	if (deleteTreeWhenFinished && tree!=NULL)
		delete this->tree;
	this->tree=tree;
	this->deleteTreeWhenFinished=deleteLater;
	model=new GeneItemModel(tree);
	resultTree->setModel(model);
	try{
		graphDisplay->setPixmap(QPixmap(imageFileName));
		graphArea->setWidget(graphDisplay);
	} catch(...) {}
}

