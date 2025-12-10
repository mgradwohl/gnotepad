#include "IncludeOrderTests.h"

#include <QtCore/QDir>
#include <QtCore/QFile>
#include <QtCore/QFileInfo>
#include <QtCore/QRegularExpression>
#include <QtCore/QTextStream>
#include <QtTest/QtTest>

IncludeOrderTests::IncludeOrderTests(QObject* parent) : QObject(parent)
{
}

IncludeOrderTests::IncludeCategory IncludeOrderTests::categorizeInclude(const QString& includeLine,
                                                                         const QString& expectedMatchingHeader)
{
    // Extract the include path from #include "..." or #include <...>
    QRegularExpression includeRegex(R"(^\s*#include\s+[<"]([^">]+)[">])");
    QRegularExpressionMatch match = includeRegex.match(includeLine);

    if (!match.hasMatch())
    {
        return Other;
    }

    QString includePath = match.captured(1);

    // 1. Matching header (only for .cpp files)
    if (!expectedMatchingHeader.isEmpty() && includePath.endsWith(expectedMatchingHeader))
    {
        return MatchingHeader;
    }

    // 2. Project headers (src/, include/, tests/)
    if (includePath.startsWith("src/") || includePath.startsWith("include/") || includePath.startsWith("tests/") ||
        includePath.contains("ui/") || includePath.contains("app/"))
    {
        return ProjectHeaders;
    }

    // 3. Third-party non-Qt (spdlog, fmt, boost)
    if (includePath.startsWith("spdlog/") || includePath.startsWith("fmt/") || includePath.startsWith("boost/"))
    {
        return ThirdPartyNonQt;
    }

    // 4. Qt headers (Qt*/... or q*.h)
    if (includePath.startsWith("Qt") || includePath.startsWith("q") || includePath.contains("/q"))
    {
        return QtHeaders;
    }

    // 5. C++ standard library (no slashes, just <name>)
    if (!includePath.contains('/'))
    {
        return StdHeaders;
    }

    // 6. Everything else
    return Other;
}

bool IncludeOrderTests::validateIncludeOrder(const QString& filePath, const QString& expectedMatchingHeader)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        qWarning() << "Failed to open file:" << filePath;
        return false;
    }

    QTextStream in(&file);
    QVector<QPair<int, IncludeCategory>> includeOrder;
    int lineNumber = 0;
    int lastCategory = 0;
    bool inIncludeSection = false;
    bool hasSeenNonInclude = false;

    while (!in.atEnd())
    {
        QString line = in.readLine();
        lineNumber++;

        // Skip empty lines and comments
        QString trimmed = line.trimmed();
        if (trimmed.isEmpty() || trimmed.startsWith("//"))
        {
            continue;
        }

        // Check if this is an include line
        if (trimmed.startsWith("#include"))
        {
            inIncludeSection = true;
            IncludeCategory category = categorizeInclude(line, expectedMatchingHeader);
            includeOrder.append({lineNumber, category});

            // Check if category is increasing (with allowance for same category)
            if (static_cast<int>(category) < lastCategory)
            {
                qWarning() << "Include order violation at line" << lineNumber << "in" << filePath;
                qWarning() << "Category" << category << "comes after category" << lastCategory;
                qWarning() << "Line:" << line;
                return false;
            }

            lastCategory = static_cast<int>(category);
        }
        else if (trimmed.startsWith("#pragma") || trimmed.startsWith("#ifndef") || trimmed.startsWith("#define") ||
                 trimmed.startsWith("#endif"))
        {
            // Header guards and pragmas are OK
            continue;
        }
        else if (inIncludeSection && !trimmed.isEmpty())
        {
            // We've seen includes and now we see something else
            hasSeenNonInclude = true;
        }
    }

    file.close();
    return true;
}

void IncludeOrderTests::testSampleSourceFileIncludeOrder()
{
    QString projectRoot = QDir::currentPath();
    while (!QFileInfo::exists(projectRoot + "/src") && projectRoot != "/")
    {
        projectRoot = QDir(projectRoot).filePath("..");
        projectRoot = QDir(projectRoot).canonicalPath();
    }

    // Test a sample .cpp file (Application.cpp)
    QString filePath = projectRoot + "/src/app/Application.cpp";
    if (QFileInfo::exists(filePath))
    {
        QVERIFY2(validateIncludeOrder(filePath, "Application.h"),
                 qPrintable(QString("Include order validation failed for %1").arg(filePath)));
    }
}

void IncludeOrderTests::testMainWindowIncludeOrder()
{
    QString projectRoot = QDir::currentPath();
    while (!QFileInfo::exists(projectRoot + "/src") && projectRoot != "/")
    {
        projectRoot = QDir(projectRoot).filePath("..");
        projectRoot = QDir(projectRoot).canonicalPath();
    }

    QString filePath = projectRoot + "/src/ui/MainWindow.cpp";
    if (QFileInfo::exists(filePath))
    {
        QVERIFY2(validateIncludeOrder(filePath, "MainWindow.h"),
                 qPrintable(QString("Include order validation failed for %1").arg(filePath)));
    }
}

void IncludeOrderTests::testTextEditorIncludeOrder()
{
    QString projectRoot = QDir::currentPath();
    while (!QFileInfo::exists(projectRoot + "/src") && projectRoot != "/")
    {
        projectRoot = QDir(projectRoot).filePath("..");
        projectRoot = QDir(projectRoot).canonicalPath();
    }

    QString filePath = projectRoot + "/src/ui/TextEditor.cpp";
    if (QFileInfo::exists(filePath))
    {
        QVERIFY2(validateIncludeOrder(filePath, "TextEditor.h"),
                 qPrintable(QString("Include order validation failed for %1").arg(filePath)));
    }
}
