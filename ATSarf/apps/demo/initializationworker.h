#ifndef INITIALIZATIONWORKER_H
#define INITIALIZATIONWORKER_H

#include <QObject>
#include <QString>
#include <ATMProgressIFC.h>

class InitializationWorker : public QObject {
        Q_OBJECT
    public:
        explicit InitializationWorker(QString *output_str, QString *error_str, ATMProgressIFC *progress, QObject *parent = 0);
        virtual ~InitializationWorker();

    signals:
        void finished();

    public slots:
        void run();

    private:
        QString *m_output_str;
        QString *m_error_str;
        ATMProgressIFC *m_progress;
};

#endif // INITIALIZATIONWORKER_H
