#include "initializationworker.h"

#include <QDebug>
#include <QThread>
#include <QMessageBox>

#include <hadith.h>
#include <hadithCommon.h>

InitializationWorker::InitializationWorker(QString *output_str, QString *error_str, ATMProgressIFC *progress, QObject *parent)
    : QObject(parent), m_output_str(output_str), m_error_str(error_str), m_progress(progress) {}

InitializationWorker::~InitializationWorker() {}

void InitializationWorker::run() {
    theSarf = new Sarf();

    if (!theSarf->start(m_output_str, m_error_str, m_progress)) {
        QMessageBox::critical(NULL, tr("ATMineDemo"), tr("Cannot set up the project. The application will now exit."));
    }

    Sarf::use(theSarf);
    initialize_other();
    hadith_initialize();
    geneology_initialize();
    emit finished();
}

