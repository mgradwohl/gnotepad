#pragma once

#include <QObject>
#include <QtCore/QCommandLineParser>

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

private:
    void setupParser(QCommandLineParser& parser);
};
