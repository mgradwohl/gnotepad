#pragma once

#include <QObject>

class IncludeOrderTests : public QObject
{
    Q_OBJECT

public:
    explicit IncludeOrderTests(QObject* parent = nullptr);

private slots:
    void testSampleSourceFileIncludeOrder();
    void testMainWindowIncludeOrder();
    void testTextEditorIncludeOrder();

private:
    enum IncludeCategory
    {
        MatchingHeader = 1,
        ProjectHeaders = 2,
        ThirdPartyNonQt = 3,
        QtHeaders = 4,
        StdHeaders = 5,
        Other = 6
    };

    IncludeCategory categorizeInclude(const QString& includeLine, const QString& expectedMatchingHeader);
    bool validateIncludeOrder(const QString& filePath, const QString& expectedMatchingHeader = QString());
};
