#pragma once

#include <QObject>
#include <QString>

namespace GnotePad::ui
{
    class MainWindow;
}

class ReliabilityTests : public QObject
{
    Q_OBJECT

public:
    explicit ReliabilityTests(QObject* parent = nullptr);

private slots:
    void initTestCase();
    void cleanupTestCase();

    // MRU (Most Recently Used) file handling tests
    void testMruPersistenceAcrossRestarts();
    void testMruUpdateOnFileOpen();
    void testMruCleanupNonExistentFiles();
    void testMruMaximumSizeEnforcement();
    void testMruDuplicateEntries();
    void testMruFileOrdering();

    // QSettings persistence and recovery tests
    void testSettingsPersistenceAcrossRestarts();
    void testSettingsRecoveryFromMissingFile();
    void testSettingsRecoveryFromCorruptData();
    void testWindowGeometryPersistence();
    void testEditorStatePersistence();
    void testEncodingPreferencePersistence();

    // Error scenario coverage tests
    void testOpenFilePermissionDenied();
    void testSaveFilePermissionDenied();
    void testSaveToReadOnlyFile();
    void testSaveToInvalidPath();
    void testLoadNonExistentFile();
    void testLoadBinaryFile();
    void testSaveWithInsufficientSpace();

    // Window state and stability tests
    void testWindowPositionPersistence();
    void testWindowStateStability();
    void testRapidWindowStateChanges();
    void testCloseWithUnsavedChanges();
    void testMultipleWindowStateCycles();
    void testMinimizeRestoreContent();
    void testMaximizeRestoreContent();

private:
    QString resolveTestFile(const QString& name) const;
    QString createTempFile(const QString& content = QString()) const;
    void clearAllSettings();
    bool makeFileReadOnly(const QString& path);
    bool makeFileWritable(const QString& path);
    bool removeFilePermissions(const QString& path, bool read, bool write);

    QString m_tempDir;
};
