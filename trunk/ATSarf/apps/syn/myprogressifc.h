#ifndef MYPROGRESSIFC_H
#define MYPROGRESSIFC_H
#include<QtGui/QColor>
/**
  * @class MyProgressIFC
  * @brief This class inherets from EmptyProgressIFC and implements the different functions present in it. Those functions
  * are used in order to expose the progress of the program we implemented. For detailed description, refer to the
  * documentation of the class.
  */
class MyProgressIFC : public ATMProgressIFC {

public:
    virtual void report(int value) {
    }

    virtual void startTaggingText(QString & text) {
    }

    virtual void tag(int start, int length,QColor color, bool textcolor=true) {
    }

    virtual void finishTaggingText() {
    }

    virtual void setCurrentAction(const QString & s) {
    }

    virtual void resetActionDisplay() {
    }

    virtual QString getFileName() {
        return "";
    }
    virtual void displayGraph(AbstractGraph *) {}
};
#endif
