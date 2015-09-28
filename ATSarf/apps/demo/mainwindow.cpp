#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QDebug>
#include <QMessageBox>
#include <QThread>
#include <QThreadPool>
#include <QTimer>

#include <hadith.h>
#include <hadithCommon.h>

#include "initializationworker.h"


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow) {
    ui->setupUi(this);
    setUnifiedTitleAndToolBarOnMac(true);
    ui->statusbar->showMessage("Idle.");
    connect(this, &MainWindow::updateProgress, ui->statusbar->progressBar(), &QProgressBar::setValue);
    connect(this, &MainWindow::resetProgress, &MainWindow::on_resetProgress);
    connect(this, SIGNAL(actionUpdate(const QString &)), ui->statusbar, SLOT(showMessage(const QString &)));
    QTimer::singleShot(100, this, SLOT(on_initializeSystem));
}

MainWindow::~MainWindow() {
    delete ui;
}

void MainWindow::changeEvent(QEvent *e) {
    QMainWindow::changeEvent(e);

    switch (e->type()) {
        case QEvent::LanguageChange:
            ui->retranslateUi(this);
            break;

        default:
            break;
    }
}

void MainWindow::on_resetProgress() {
    emit updateProgress(0);
    emit actionUpdate("Idle.");
}

void MainWindow::report(int value) {
    emit updateProgress(value);
}

void MainWindow::setCurrentAction(const QString &action) {
    emit actionUpdate(action);
}

void MainWindow::resetActionDisplay() {
    emit resetProgress();
}

void MainWindow::tag(int /*start*/, int /*length*/, QColor /*color*/, bool /*textcolor*/) {
//    QTextBrowser *taggedBox = m_ui->hadith_display;
//    QTextCursor c = taggedBox->textCursor();
//    c.setPosition(start, QTextCursor::MoveAnchor);
//    c.setPosition(start + length, QTextCursor::KeepAnchor);
//    taggedBox->setTextCursor(c);
//    if (textcolor) {
//        taggedBox->setTextColor(color);
//    } else {
//        taggedBox->setTextBackgroundColor(color);
//    }
}

void MainWindow::startTaggingText(QString &/*text*/) {
//    QTextBrowser *taggedBox = m_ui->hadith_display;
//    taggedBox->clear();
//    taggedBox->setLayoutDirection(Qt::RightToLeft);
//    QTextCursor c = taggedBox->textCursor();
//    c.clearSelection();
//    c.movePosition(QTextCursor::Start, QTextCursor::MoveAnchor);
//    taggedBox->setTextCursor(c);
//    taggedBox->setTextBackgroundColor(Qt::white);
//    taggedBox->setTextColor(Qt::black);
//    taggedBox->setText(text);
}

void MainWindow::finishTaggingText() {
//    QTextBrowser *taggedBox = m_ui->hadith_display;
//    QTextCursor c = taggedBox->textCursor();
//    c.movePosition(QTextCursor::End, QTextCursor::MoveAnchor);
//    taggedBox->setTextCursor(c);
}

QString MainWindow::getFileName() {
//    if (browseFileDlg == NULL) {
//        QString dir = QDir::currentPath();
//        browseFileDlg = new QFileDialog(NULL, QString("Open File"), dir, QString("All Files (*)"));
//        browseFileDlg->setOptions(QFileDialog::DontUseNativeDialog);
//        browseFileDlg->setFileMode(QFileDialog::ExistingFile);
//        browseFileDlg->setViewMode(QFileDialog::Detail);
//    }
//    if (browseFileDlg->exec()) {
//        QStringList files = browseFileDlg->selectedFiles();
//        QString fileName = files[0];
//        return fileName;
//    }
    return "";
}

void MainWindow::displayGraph(AbstractGraph */*graph*/) {
//    try {
//        system("dot -Tsvg graph.dot -o graph.svg");
//        if (graph == NULL) {
//            QMainWindow *mw = new QMainWindow(NULL);
//            mw->setWindowTitle(QString("Sarf Graph (") + m_ui->input->toPlainText() + ")");
//            QScrollArea *sa = new QScrollArea(mw);
//            mw->setCentralWidget(sa);
//            QLabel *pic = new QLabel(sa);
//            pic->setPixmap(QPixmap("./graph.svg"));
//            sa->setWidget(pic);
//            mw->show();
//        } else {
//            GeneMainWindow *m = new GeneMainWindow;
//            m->show();
//            GeneTree *t = (dynamic_cast<GeneTree *>(graph))->duplicate();
//            m->display(t, "./graph.svg", true);
//        }
//    } catch (...) {
//    }
}

void MainWindow::on_initializeSystem() {
    emit actionUpdate("Initializing...");
    QThread *thread = new QThread();
    InitializationWorker *worker = new InitializationWorker(&output_str, &error_str, this);
    connect(thread, &QThread::started, worker, &InitializationWorker::run);
    connect(worker, &InitializationWorker::finished, thread, &QThread::quit);
    connect(worker, &InitializationWorker::finished, worker, &QObject::deleteLater);
    connect(thread, &QThread::finished, thread, &QObject::deleteLater);
    worker->moveToThread(thread);
    thread->start();
}
