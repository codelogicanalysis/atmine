#include "initializationworker.h"

#include <QDebug>
#include <QThread>

#include <hadith.h>
#include <hadithCommon.h>

InitializationWorker::InitializationWorker(QObject *parent) : QObject(parent) {}

InitializationWorker::~InitializationWorker() {}

void InitializationWorker::run() {
    qDebug() << this->thread()->currentThreadId();
    initialize_other();
    hadith_initialize();
    geneology_initialize();
    emit finished();
}

