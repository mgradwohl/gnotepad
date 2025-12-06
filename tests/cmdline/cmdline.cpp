#include "ApplicationCmdLineTests.h"

#include <QtCore/QCommandLineParser>
#include <QtCore/QStringList>
#include <QtTest/QTest>

ApplicationCmdLineTests::ApplicationCmdLineTests(QObject* parent)
    : QObject(parent)
{}

void ApplicationCmdLineTests::testQuitAfterInitParsing()
{
    // Test that --quit-after-init flag is recognized by QCommandLineParser
    QCommandLineParser parser;
    parser.setSingleDashWordOptionMode(QCommandLineParser::ParseAsLongOptions);
    
    QCommandLineOption quitAfterInitOption(
        {QStringLiteral("quit-after-init"), QStringLiteral("headless-smoke")},
        QStringLiteral("Quit shortly after startup (useful for headless smoke tests)."));
    
    parser.addOption(quitAfterInitOption);
    
    QStringList args;
    args << QStringLiteral("GnotePad") << QStringLiteral("--quit-after-init");
    
    QVERIFY(parser.parse(args));
    QVERIFY(parser.isSet(quitAfterInitOption));
}

void ApplicationCmdLineTests::testHeadlessSmokeParsing()
{
    // Test that --headless-smoke alias works
    QCommandLineParser parser;
    parser.setSingleDashWordOptionMode(QCommandLineParser::ParseAsLongOptions);
    
    QCommandLineOption quitAfterInitOption(
        {QStringLiteral("quit-after-init"), QStringLiteral("headless-smoke")},
        QStringLiteral("Quit shortly after startup (useful for headless smoke tests)."));
    
    parser.addOption(quitAfterInitOption);
    
    QStringList args;
    args << QStringLiteral("GnotePad") << QStringLiteral("--headless-smoke");
    
    QVERIFY(parser.parse(args));
    QVERIFY(parser.isSet(quitAfterInitOption));
}

void ApplicationCmdLineTests::testNoFlagsParsing()
{
    // Test normal startup without flags
    QCommandLineParser parser;
    parser.setSingleDashWordOptionMode(QCommandLineParser::ParseAsLongOptions);
    
    QCommandLineOption quitAfterInitOption(
        {QStringLiteral("quit-after-init"), QStringLiteral("headless-smoke")},
        QStringLiteral("Quit shortly after startup (useful for headless smoke tests)."));
    
    parser.addOption(quitAfterInitOption);
    
    QStringList args;
    args << QStringLiteral("GnotePad");
    
    QVERIFY(parser.parse(args));
    QVERIFY(!parser.isSet(quitAfterInitOption));
}

void ApplicationCmdLineTests::testQuitAfterInitBehavior()
{
    // Test that both option names work and set the same flag
    QCommandLineParser parser1;
    parser1.setSingleDashWordOptionMode(QCommandLineParser::ParseAsLongOptions);
    
    QCommandLineOption quitOption1(
        {QStringLiteral("quit-after-init"), QStringLiteral("headless-smoke")},
        QStringLiteral("Quit shortly after startup (useful for headless smoke tests)."));
    
    parser1.addOption(quitOption1);
    
    QStringList args1;
    args1 << QStringLiteral("GnotePad") << QStringLiteral("--quit-after-init");
    
    QVERIFY(parser1.parse(args1));
    QVERIFY(parser1.isSet(quitOption1));
    
    // Verify the second alias also works
    QCommandLineParser parser2;
    parser2.setSingleDashWordOptionMode(QCommandLineParser::ParseAsLongOptions);
    
    QCommandLineOption quitOption2(
        {QStringLiteral("quit-after-init"), QStringLiteral("headless-smoke")},
        QStringLiteral("Quit shortly after startup (useful for headless smoke tests)."));
    
    parser2.addOption(quitOption2);
    
    QStringList args2;
    args2 << QStringLiteral("GnotePad") << QStringLiteral("--headless-smoke");
    
    QVERIFY(parser2.parse(args2));
    QVERIFY(parser2.isSet(quitOption2));
}

int main(int argc, char** argv)
{
    ApplicationCmdLineTests tc;
    return QTest::qExec(&tc, argc, argv);
}

