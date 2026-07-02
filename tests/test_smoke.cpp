#include <QtTest/QtTest>

class SmokeTest : public QObject
{
    Q_OBJECT

private slots:
    void testEnvironment()
    {
        QVERIFY(true);
        QCOMPARE(1 + 1, 2);
    }
};

QTEST_MAIN(SmokeTest)
#include "test_smoke.moc"