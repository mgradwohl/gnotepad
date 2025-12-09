#pragma once

#include <QObject>
#include <QtWidgets/qapplication.h>

class StyleConfigTests : public QObject
{
    Q_OBJECT

public:
    explicit StyleConfigTests(QObject* parent = nullptr);

private slots:
    void testStyleFactoryKeysNotEmpty();
    void testFusionStyleAvailable();
    void testWindowsStyleAvailable();
    void testStyleSelectionDoesNotCrash();
};
