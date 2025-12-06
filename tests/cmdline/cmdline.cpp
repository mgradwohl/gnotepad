#include "ApplicationCmdLineTests.h"

#include <QtCore/QCommandLineOption>
#include <QtCore/QStringList>
#include <QtTest/QTest>

ApplicationCmdLineTests::ApplicationCmdLineTests(QObject* parent)
    : QObject(parent)
{}

void ApplicationCmdLineTests::setupParser(QCommandLineParser& parser)
{
    parser.setSingleDashWordOptionMode(QCommandLineParser::ParseAsLongOptions);
    
    QCommandLineOption quitAfterInitOption(
        {QStringLiteral("quit-after-init"), QStringLiteral("headless-smoke")},
        QStringLiteral("Quit shortly after startup (useful for headless smoke tests)."));
    
    parser.addOption(quitAfterInitOption);
}

void ApplicationCmdLineTests::testQuitAfterInitParsing()
{
    // Test that --quit-after-init flag is recognized by QCommandLineParser
    QCommandLineParser parser;
    setupParser(parser);
    
    QStringList args;
    args << QStringLiteral("GnotePad") << QStringLiteral("--quit-after-init");
    
    QVERIFY(parser.parse(args));
    QVERIFY(parser.isSet(QStringLiteral("quit-after-init")));
}

void ApplicationCmdLineTests::testHeadlessSmokeParsing()
{
    // Test that --headless-smoke alias works
    QCommandLineParser parser;
    setupParser(parser);
    
    QStringList args;
    args << QStringLiteral("GnotePad") << QStringLiteral("--headless-smoke");
    
    QVERIFY(parser.parse(args));
    QVERIFY(parser.isSet(QStringLiteral("headless-smoke")));
}

void ApplicationCmdLineTests::testNoFlagsParsing()
{
    // Test normal startup without flags
    QCommandLineParser parser;
    setupParser(parser);
    
    QStringList args;
    args << QStringLiteral("GnotePad");
    
    QVERIFY(parser.parse(args));
    QVERIFY(!parser.isSet(QStringLiteral("quit-after-init")));
}

void ApplicationCmdLineTests::testQuitAfterInitBehavior()
{
    // Test that both option names work and can be detected
    QCommandLineParser parser;
    setupParser(parser);
    
    QStringList args1;
    args1 << QStringLiteral("GnotePad") << QStringLiteral("--quit-after-init");
    
    QVERIFY(parser.parse(args1));
    QVERIFY(parser.isSet(QStringLiteral("quit-after-init")));
    
    // Verify the second alias also works - reset parser for the second test
    QCommandLineParser parser2;
    setupParser(parser2);
    
    QStringList args2;
    args2 << QStringLiteral("GnotePad") << QStringLiteral("--headless-smoke");
    
    QVERIFY(parser2.parse(args2));
    QVERIFY(parser2.isSet(QStringLiteral("headless-smoke")));
}

int main(int argc, char** argv)
{
    ApplicationCmdLineTests tc;
    return QTest::qExec(&tc, argc, argv);
}

