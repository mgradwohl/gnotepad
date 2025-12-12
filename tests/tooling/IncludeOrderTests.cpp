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

QString IncludeOrderTests::findProjectRoot() const
{
    // Start from current directory and search up for src directory
    QString searchDir = QDir::currentPath();
    constexpr int kMaxSearchDepth = 20; // Prevent infinite loop

    for (int depth = 0; depth < kMaxSearchDepth; ++depth)
    {
        if (QFileInfo::exists(searchDir + "/src") && QDir(searchDir + "/src").exists())
        {
            return searchDir;
        }

        QDir parent(searchDir);
        if (!parent.cdUp())
        {
            break; // Can't go up any more (reached root)
        }

        QString newPath = parent.canonicalPath();
        if (newPath == searchDir)
        {
            break; // Already at root
        }
        searchDir = newPath;
    }

    return QString(); // Not found
}

void IncludeOrderTests::initTestCase()
{
    m_projectRoot = findProjectRoot();
    if (m_projectRoot.isEmpty())
    {
        QFAIL("Could not find project root directory (no src directory found)");
    }
    qInfo() << "Project root:" << m_projectRoot;
}

IncludeOrderTests::IncludeCategory IncludeOrderTests::categorizeInclude(const QString& includeLine, const QString& expectedMatchingHeader)
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
    if (!expectedMatchingHeader.isEmpty() && (includePath == expectedMatchingHeader || includePath.endsWith("/" + expectedMatchingHeader)))
    {
        return MatchingHeader;
    }

    // 2. Project headers (src/, include/, tests/)
    if (includePath.startsWith("src/") || includePath.startsWith("include/") || includePath.startsWith("tests/") ||
        (includePath.startsWith("ui/") || includePath.startsWith("app/")))
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
            // We've seen includes and now we see something else - end of include section
            break;
        }
    }

    file.close();
    return true;
}

void IncludeOrderTests::testSampleSourceFileIncludeOrder()
{
    // Test a sample .cpp file (Application.cpp)
    QString filePath = m_projectRoot + "/src/app/Application.cpp";
    if (!QFileInfo::exists(filePath))
    {
        QSKIP(qPrintable(QString("Source file not found: %1").arg(filePath)));
    }
    QVERIFY2(validateIncludeOrder(filePath, "Application.h"), qPrintable(QString("Include order validation failed for %1").arg(filePath)));
}

void IncludeOrderTests::testMainWindowIncludeOrder()
{
    QString filePath = m_projectRoot + "/src/ui/MainWindow.cpp";
    if (!QFileInfo::exists(filePath))
    {
        QSKIP(qPrintable(QString("Source file not found: %1").arg(filePath)));
    }
    QVERIFY2(validateIncludeOrder(filePath, "MainWindow.h"), qPrintable(QString("Include order validation failed for %1").arg(filePath)));
}

void IncludeOrderTests::testTextEditorIncludeOrder()
{
    QString filePath = m_projectRoot + "/src/ui/TextEditor.cpp";
    if (!QFileInfo::exists(filePath))
    {
        QSKIP(qPrintable(QString("Source file not found: %1").arg(filePath)));
    }
    QVERIFY2(validateIncludeOrder(filePath, "TextEditor.h"), qPrintable(QString("Include order validation failed for %1").arg(filePath)));
}
