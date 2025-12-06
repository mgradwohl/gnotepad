#pragma once

#include <QObject>

namespace GnotePad
{
class Application;
}

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
};
