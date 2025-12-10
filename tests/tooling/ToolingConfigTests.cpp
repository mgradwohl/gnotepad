#include "ToolingConfigTests.h"

#include <QtCore/QDir>
#include <QtCore/QFile>
#include <QtCore/QFileInfo>
#include <QtCore/QProcess>
#include <QtCore/QTextStream>
#include <QtTest/QtTest>

ToolingConfigTests::ToolingConfigTests(QObject* parent) : QObject(parent)
{
}

void ToolingConfigTests::testClangTidyConfigExists()
{
    QString projectRoot = QDir::currentPath();
    while (!QFileInfo::exists(projectRoot + "/.clang-tidy") && projectRoot != "/")
    {
        projectRoot = QDir(projectRoot).filePath("..");
        projectRoot = QDir(projectRoot).canonicalPath();
    }

    QFileInfo clangTidyFile(projectRoot + "/.clang-tidy");
    QVERIFY2(clangTidyFile.exists(), ".clang-tidy configuration file must exist in project root");
    QVERIFY2(clangTidyFile.isFile(), ".clang-tidy must be a regular file");
    QVERIFY2(clangTidyFile.isReadable(), ".clang-tidy must be readable");
}

void ToolingConfigTests::testClangTidyConfigIsValid()
{
    QString projectRoot = QDir::currentPath();
    while (!QFileInfo::exists(projectRoot + "/.clang-tidy") && projectRoot != "/")
    {
        projectRoot = QDir(projectRoot).filePath("..");
        projectRoot = QDir(projectRoot).canonicalPath();
    }

    QFile file(projectRoot + "/.clang-tidy");
    QVERIFY(file.open(QIODevice::ReadOnly | QIODevice::Text));

    QTextStream in(&file);
    QString content = in.readAll();
    file.close();

    // Verify essential configuration elements are present
    QVERIFY2(content.contains("Checks:"), ".clang-tidy must contain 'Checks:' section");
    QVERIFY2(content.contains("HeaderFilterRegex:"), ".clang-tidy must contain 'HeaderFilterRegex:' section");
    QVERIFY2(content.contains("FormatStyle:"), ".clang-tidy must contain 'FormatStyle:' section");
}

void ToolingConfigTests::testClangFormatConfigExists()
{
    QString projectRoot = QDir::currentPath();
    while (!QFileInfo::exists(projectRoot + "/.clang-format") && projectRoot != "/")
    {
        projectRoot = QDir(projectRoot).filePath("..");
        projectRoot = QDir(projectRoot).canonicalPath();
    }

    QFileInfo clangFormatFile(projectRoot + "/.clang-format");
    QVERIFY2(clangFormatFile.exists(), ".clang-format configuration file must exist in project root");
    QVERIFY2(clangFormatFile.isFile(), ".clang-format must be a regular file");
    QVERIFY2(clangFormatFile.isReadable(), ".clang-format must be readable");
}

void ToolingConfigTests::testClangFormatConfigIsValid()
{
    QString projectRoot = QDir::currentPath();
    while (!QFileInfo::exists(projectRoot + "/.clang-format") && projectRoot != "/")
    {
        projectRoot = QDir(projectRoot).filePath("..");
        projectRoot = QDir(projectRoot).canonicalPath();
    }

    QFile file(projectRoot + "/.clang-format");
    QVERIFY(file.open(QIODevice::ReadOnly | QIODevice::Text));

    QTextStream in(&file);
    QString content = in.readAll();
    file.close();

    // Verify essential configuration elements for include ordering
    QVERIFY2(content.contains("BasedOnStyle:"), ".clang-format must contain 'BasedOnStyle:' section");
    QVERIFY2(content.contains("SortIncludes:"), ".clang-format must contain 'SortIncludes:' section");
    QVERIFY2(content.contains("IncludeBlocks:"), ".clang-format must contain 'IncludeBlocks:' section");
    QVERIFY2(content.contains("IncludeCategories:"), ".clang-format must contain 'IncludeCategories:' section");

    // Verify include ordering is enabled
    QVERIFY2(!content.contains("SortIncludes: false") && !content.contains("SortIncludes: Never"),
             "Include sorting must be enabled in .clang-format");
}

void ToolingConfigTests::testIncludeCleanerExclusionsPresent()
{
    QString projectRoot = QDir::currentPath();
    while (!QFileInfo::exists(projectRoot + "/.clang-tidy") && projectRoot != "/")
    {
        projectRoot = QDir(projectRoot).filePath("..");
        projectRoot = QDir(projectRoot).canonicalPath();
    }

    QFile file(projectRoot + "/.clang-tidy");
    QVERIFY(file.open(QIODevice::ReadOnly | QIODevice::Text));

    QTextStream in(&file);
    QString content = in.readAll();
    file.close();

    // Verify include-cleaner exclusions are configured for external libraries
    QVERIFY2(content.contains("misc-include-cleaner.IgnoreHeaders"),
             ".clang-tidy must configure misc-include-cleaner.IgnoreHeaders to exclude external libraries");

    // Verify Qt and spdlog are excluded
    QVERIFY2(content.contains("qt6") || content.contains("Qt"),
             "Qt headers should be excluded from include-cleaner checks");
    QVERIFY2(content.contains("spdlog"), "spdlog headers should be excluded from include-cleaner checks");
}
