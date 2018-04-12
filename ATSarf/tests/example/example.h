#ifndef EXAMPLE_H
#define EXAMPLE_H

#include <QtTest/QtTest>

class TestExample: public QObject {
        Q_OBJECT

    private slots:
        void initTestCase();
        void firstTest();
        void secondTest();
        void cleanupTestCase();
};

#endif // EXAMPLE_H

