#pragma once

#include <QObject>
#include <QString>

class ToolingConfigTests : public QObject
{
    Q_OBJECT

public:
    explicit ToolingConfigTests(QObject* parent = nullptr);

private slots:
    void initTestCase();
    void testClangTidyConfigExists();
    void testClangTidyConfigIsValid();
    void testClangFormatConfigExists();
    void testClangFormatConfigIsValid();
    void testIncludeCleanerExclusionsPresent();

private:
    QString findProjectRoot() const;
    QString m_projectRoot;
};
