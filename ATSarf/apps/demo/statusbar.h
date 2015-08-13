#ifndef STATUSBAR_H
#define STATUSBAR_H

#include <QWidget>
#include <QStatusBar>
#include <QProgressBar>

class StatusBar : public QStatusBar {
        Q_OBJECT
    public:
        explicit StatusBar(QWidget *parent = 0);
        virtual ~StatusBar();

        const QProgressBar *progressBar() const;
    public slots:
        void on_updateProgress(int progress);
        void on_resetDisplay();

    private:
        QProgressBar *m_progressBar;
};

#endif // STATUSBAR_H
