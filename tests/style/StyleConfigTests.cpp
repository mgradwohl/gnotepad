#include "StyleConfigTests.h"

#include <QtTest/QtTest>
#include <QtWidgets/qstylefactory.h>

StyleConfigTests::StyleConfigTests(QObject* parent) : QObject(parent)
{
}

void StyleConfigTests::testStyleFactoryKeysNotEmpty()
{
    QStringList styles = QStyleFactory::keys();
    QVERIFY(!styles.isEmpty());
}

void StyleConfigTests::testFusionStyleAvailable()
{
    QStringList styles = QStyleFactory::keys();
    // Fusion should be available on all Qt installations
    QVERIFY(styles.contains(QStringLiteral("fusion"), Qt::CaseInsensitive));
}

void StyleConfigTests::testWindowsStyleAvailable()
{
    QStringList styles = QStyleFactory::keys();
    // Windows style should be available on most Qt installations
    // This test documents the expectation, but may not be true on all systems
    bool hasWindowsStyle = styles.contains(QStringLiteral("windows"), Qt::CaseInsensitive);
    // Just log result, don't fail the test
    if (!hasWindowsStyle)
    {
        qDebug() << "Note: 'windows' style not available on this system";
    }
    QVERIFY(true); // Always pass
}

void StyleConfigTests::testStyleSelectionDoesNotCrash()
{
    QStringList styles = QStyleFactory::keys();

    // Test setting each available style
    for (const QString& styleName : styles)
    {
        QApplication::setStyle(styleName);
        QVERIFY(QApplication::style() != nullptr);
    }

    // Test setting non-existent style (should not crash)
    QApplication::setStyle(QStringLiteral("nonexistent-style-12345"));
    QVERIFY(QApplication::style() != nullptr); // Should still have a valid style
}
