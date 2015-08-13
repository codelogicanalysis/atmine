#include "statusbar.h"

StatusBar::StatusBar(QWidget *parent) :
    QStatusBar(parent),
    m_progressBar(new QProgressBar(this)) {
    setSizeGripEnabled(false);
    m_progressBar->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    m_progressBar->setMinimum(0);
    m_progressBar->setMaximum(100);
    m_progressBar->setTextVisible(false);
    this->addPermanentWidget(m_progressBar);
}

StatusBar::~StatusBar() {}

const QProgressBar * StatusBar::progressBar() const {
    return m_progressBar;
}

void StatusBar::on_updateProgress(int progress) {
}

void StatusBar::on_resetDisplay() {
}
