#pragma once

#include <QtWidgets/qapplication.h>

#include <QObject>

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
