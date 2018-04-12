#include <QMainWindow>
#include <QtGui>
#include <QApplication>
class ATTrie;

class ATMTrieTestMainWindow : public QMainWindow {
    Q_OBJECT
    public:
        ATTrie * trie;
        QPlainTextEdit * textEdit;
        QString curFile;

        QAction * newAct, * openAct, * saveAct, * saveAsAct, * exitAct;
        QAction * cutAct, * copyAct, * pasteAct, * aboutAct, * aboutQtAct;

        QMenu * fileMenu,* editMenu, *helpMenu;
        QToolBar *  fileToolBar, * editToolBar;

        ATMTrieTestMainWindow ();
    protected:
        void closeEvent(QCloseEvent *event);
    private slots:
        void newFile();
        void open();
        bool save();
        bool saveAs();
        void about();
        void documentWasModified();
    private:
        void createActions();
        void createMenus();
        void createToolBars();
        void createStatusBar();
        void readSettings();
        void writeSettings();
        bool maybeSave();
        void loadFile(const QString &fileName);
        bool saveFile(const QString &fileName);
        void setCurrentFile(const QString &fileName);
        QString strippedName(const QString &fullFileName);
};
