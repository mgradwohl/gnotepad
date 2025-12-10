#pragma once

#include <QObject>

class ToolingConfigTests : public QObject
{
    Q_OBJECT

public:
    explicit ToolingConfigTests(QObject* parent = nullptr);

private slots:
    void testClangTidyConfigExists();
    void testClangTidyConfigIsValid();
    void testClangFormatConfigExists();
    void testClangFormatConfigIsValid();
    void testIncludeCleanerExclusionsPresent();
};
