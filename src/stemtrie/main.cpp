#include <atmTrieTest.h>

int 
main(int argc, char *argv[])
{
    Q_INIT_RESOURCE(stemtrie);

    QApplication app(argc, argv);
    app.setOrganizationName("ATM");
    app.setApplicationName("ATM Trie Test");
    ATMTrieTestMainWindow mainWin;
    mainWin.show();
    return app.exec();
}
