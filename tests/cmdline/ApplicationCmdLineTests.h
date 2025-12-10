#pragma once

#include <QtCore/QCommandLineParser>

#include <QObject>

class ApplicationCmdLineTests : public QObject
{
    Q_OBJECT

public:
    explicit ApplicationCmdLineTests(QObject* parent = nullptr);

private slots:
    void testQuitAfterInitParsing();
    void testHeadlessSmokeParsing();
    void testNoFlagsParsing();
    void testQuitAfterInitBehavior();
    void testHelpOption();
    void testVersionOption();
    void testInvalidOption();

private:
    void setupParser(QCommandLineParser& parser);
};
