#ifndef INITIALIZATIONWORKER_H
#define INITIALIZATIONWORKER_H

#include <QObject>

class InitializationWorker : public QObject
{
    Q_OBJECT
public:
    explicit InitializationWorker(QObject *parent = 0);
    virtual ~InitializationWorker();

signals:
    void finished();

public slots:
    void run();
};

#endif // INITIALIZATIONWORKER_H
