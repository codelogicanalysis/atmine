#include "example.h"

void TestExample::initTestCase() {
    // Initializing test case.
}

void TestExample::firstTest() {
    // Testing something.
    QVERIFY(true);
}

void TestExample::secondTest() {
    // Testing something else.
    QVERIFY(true);
}

void TestExample::cleanupTestCase() {
    // Cleaning up test case.
}

QTEST_MAIN(TestExample)
