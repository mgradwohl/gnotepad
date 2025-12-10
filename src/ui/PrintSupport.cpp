#include "ui/PrintSupport.h"

#include <algorithm>
#include <cmath>
#include <memory>

#include <QtCore/qcoreapplication.h>
#include <QtCore/qobject.h>
#include <QtCore/qstring.h>
#include <QtGui/qabstracttextdocumentlayout.h>
#include <QtGui/qfontmetrics.h>
#include <QtGui/qpagelayout.h>
#include <QtGui/qpagesize.h>
#include <QtGui/qpainter.h>
#include <QtGui/qtextdocument.h>
#include <QtGui/qtextlayout.h>
#include <QtGui/qtextobject.h>
#include <QtGui/qtextoption.h>
#include <QtPrintSupport/qprinter.h>
#include <QtPrintSupport/qprintpreviewdialog.h>
#include <QtWidgets/qdialog.h>
#include <QtWidgets/qwidget.h>

#include "ui/TextEditor.h"

namespace GnotePad::ui::PrintSupport
{
namespace
{

// ============================================================================
// Constants
// ============================================================================

constexpr qreal DefaultMarginMm = 12.7;       // Margin in millimeters (~0.5 inch)
constexpr qreal PointsPerInch = 72.0;         // Standard typographic points per inch
constexpr qreal GutterPaddingPt = 6.0;        // Padding around line numbers (points)
constexpr qreal HeaderFooterPaddingPt = 12.0; // Space between header/footer and content (points)

// ============================================================================
// Helper: Calculate gutter width for line numbers
// Input:  blockCount - number of text blocks in document
//         digitWidthPx - width of a single digit in PIXELS
//         gutterPaddingPx - padding in PIXELS
// Output: gutter width in PIXELS
// ============================================================================

qreal calculateGutterWidthPx(int blockCount, qreal digitWidthPx, qreal gutterPaddingPx)
{
    constexpr int LineDigitBase = 10;
    int digits = 1;
    int maxLines = std::max(1, blockCount);
    while (maxLines >= LineDigitBase)
    {
        maxLines /= LineDigitBase;
        ++digits;
    }

    return gutterPaddingPx + (digitWidthPx * digits) + gutterPaddingPx;
}

// ============================================================================
// Helper: Create a printable document copy
// Input:  editor - source text editor
//         printer - the target printer (for font metrics)
//         font - font to use (with point size)
//         textWidthPx - text area width in PIXELS
//         pageHeightPx - content area height in PIXELS
// Output: QTextDocument configured for pagination in device pixels
// ============================================================================

std::unique_ptr<QTextDocument> createPrintDocument(TextEditor* editor, QPrinter* printer, const QFont& font, qreal textWidthPx,
                                                   qreal pageHeightPx)
{
    auto doc = std::make_unique<QTextDocument>();

    doc->setDefaultFont(font);
    doc->setPlainText(editor->toPlainText());
    doc->setDocumentMargin(0.0);

    QTextOption textOption = editor->document()->defaultTextOption();
    textOption.setWrapMode(QTextOption::WrapAtWordBoundaryOrAnywhere);
    doc->setDefaultTextOption(textOption);

    // Set paint device so document uses printer DPI for layout
    if (auto* layout = doc->documentLayout())
    {
        layout->setPaintDevice(printer);
    }

    // Page size in pixels (document will use printer DPI)
    doc->setPageSize(QSizeF(textWidthPx, pageHeightPx));

    return doc;
}

// ============================================================================
// Helper: Configure printer with initial settings
// Sets up margins in millimeters via QPageLayout
// ============================================================================

void configurePrinter(QPrinter* printer, const QString& documentName)
{
    printer->setColorMode(QPrinter::GrayScale);
    printer->setFullPage(false); // Let Qt respect the margins we set
    printer->setDocName(documentName);
    printer->setCreator(QCoreApplication::applicationName());

    // Set page layout with our default margins (in millimeters)
    QPageLayout layout(QPageSize(QPageSize::Letter), QPageLayout::Portrait,
                       QMarginsF(DefaultMarginMm, DefaultMarginMm, DefaultMarginMm, DefaultMarginMm), QPageLayout::Millimeter);
    printer->setPageLayout(layout);
}

// ============================================================================
// Main render function
// Strategy: Work entirely in DEVICE PIXELS, no painter scaling
// QPrinter handles DPI, fonts stay in points, geometry in pixels
// With setFullPage(false), painter origin is at top-left of printable area
// ============================================================================

void renderDocument(TextEditor* editor, QPrinter* printer, const QString& documentName, bool includeLineNumbers)
{
    if (!printer || !editor)
        return;

    // ========================================================================
    // Step 1: Get page geometry in PIXELS
    // With setFullPage(false), paintRectPixels gives us the printable area.
    // The painter's origin (0,0) is at the top-left of this printable area.
    // ========================================================================
    const int dpi = printer->resolution();
    const qreal scale = static_cast<qreal>(dpi) / PointsPerInch; // pts to px

    const QPageLayout layout = printer->pageLayout();
    const QRectF paintRect = layout.paintRectPixels(dpi);

    // Create our working rect starting at (0,0) since painter origin is at printable area
    const QRectF pageRectPx(0, 0, paintRect.width(), paintRect.height());

    // ========================================================================
    // Step 2: Set up font and get metrics in PIXELS
    // ========================================================================
    QFont font = editor->font();
    // Ensure font has a point size (not pixel size)
    if (font.pointSizeF() <= 0 && font.pixelSize() > 0)
    {
        font.setPointSizeF(font.pixelSize() * PointsPerInch / 96.0);
    }

    // Font metrics in device pixels (for the printer)
    const QFontMetricsF metrics(font, printer);
    const qreal lineHeightPx = metrics.height();
    const qreal digitWidthPx = metrics.horizontalAdvance(QLatin1Char('9'));

    // ========================================================================
    // Step 3: Calculate layout dimensions in PIXELS
    // ========================================================================

    // Convert point constants to pixels
    const qreal gutterPaddingPx = GutterPaddingPt * scale;
    const qreal headerFooterPaddingPx = HeaderFooterPaddingPt * scale;

    // Header and footer heights
    const qreal headerHeightPx = lineHeightPx + headerFooterPaddingPx;
    const qreal footerHeightPx = lineHeightPx + headerFooterPaddingPx;

    // Content area
    const qreal contentTopPx = pageRectPx.top() + headerHeightPx;
    const qreal contentHeightPx = pageRectPx.height() - headerHeightPx - footerHeightPx;

    // Gutter for line numbers (0 if disabled)
    const qreal gutterWidthPx =
        includeLineNumbers ? calculateGutterWidthPx(editor->document()->blockCount(), digitWidthPx, gutterPaddingPx) : 0.0;

    // Text area width
    const qreal textWidthPx = pageRectPx.width() - gutterWidthPx;

    // ========================================================================
    // Step 4: Create document for pagination (in pixels)
    // ========================================================================
    auto doc = createPrintDocument(editor, printer, font, textWidthPx, contentHeightPx);
    const int totalPages = doc->pageCount();

    // ========================================================================
    // Step 5: Set up painter (NO scaling - work in device pixels)
    // ========================================================================
    QPainter painter(printer);
    painter.setRenderHint(QPainter::TextAntialiasing);
    painter.setFont(font);
    painter.setPen(Qt::black);

    // ========================================================================
    // Step 6: Render each page (all coordinates in pixels)
    // ========================================================================
    for (int pageIndex = 0; pageIndex < totalPages; ++pageIndex)
    {
        const qreal yOffsetPx = static_cast<qreal>(pageIndex) * contentHeightPx;

        // Draw header (centered filename)
        {
            const qreal textAreaLeftPx = pageRectPx.left() + gutterWidthPx;
            const qreal textAreaWidthPx = pageRectPx.width() - gutterWidthPx;
            QRectF headerRect(textAreaLeftPx, pageRectPx.top(), textAreaWidthPx, lineHeightPx);
            painter.drawText(headerRect, Qt::AlignHCenter | Qt::AlignTop, documentName);
        }

        // Draw line numbers if enabled
        if (includeLineNumbers)
        {
            QTextBlock block = doc->firstBlock();
            int blockNumber = 1;

            while (block.isValid())
            {
                const QTextLayout* blockLayout = block.layout();
                if (!blockLayout || blockLayout->lineCount() == 0)
                {
                    block = block.next();
                    ++blockNumber;
                    continue;
                }

                // Document positions are in pixels (since we set paint device)
                const QPointF blockPos = blockLayout->position();
                if (blockPos.y() > yOffsetPx + contentHeightPx)
                    break;

                const QTextLine firstLine = blockLayout->lineAt(0);
                if (firstLine.isValid())
                {
                    const qreal lineTopPx = blockPos.y() + firstLine.y();

                    // Only draw line number if the TOP of this line starts on THIS page
                    // This prevents duplicate line numbers when a line spans page breaks
                    if (lineTopPx >= yOffsetPx && lineTopPx < yOffsetPx + contentHeightPx)
                    {
                        const qreal relativeYPx = lineTopPx - yOffsetPx;
                        const qreal drawYPx = contentTopPx + relativeYPx;
                        const qreal lnHeightPx = firstLine.height();

                        painter.drawText(QRectF(pageRectPx.left(), drawYPx, gutterWidthPx - gutterPaddingPx, lnHeightPx),
                                         Qt::AlignRight | Qt::AlignVCenter, QString::number(blockNumber));
                    }
                }

                block = block.next();
                ++blockNumber;
            }
        }

        // Draw page content
        {
            painter.save();
            const QRectF contentClipPx(pageRectPx.left() + gutterWidthPx, contentTopPx, textWidthPx, contentHeightPx);
            painter.setClipRect(contentClipPx);
            painter.translate(pageRectPx.left() + gutterWidthPx, contentTopPx - yOffsetPx);
            doc->drawContents(&painter, QRectF(0.0, yOffsetPx, textWidthPx, contentHeightPx));
            painter.restore();
        }

        // Draw footer (page numbers)
        {
            const qreal textAreaLeftPx = pageRectPx.left() + gutterWidthPx;
            const qreal textAreaWidthPx = pageRectPx.width() - gutterWidthPx;
            QRectF footerRect(textAreaLeftPx, pageRectPx.bottom() - lineHeightPx, textAreaWidthPx, lineHeightPx);
            painter.drawText(footerRect, Qt::AlignRight | Qt::AlignBottom, QObject::tr("Page %1 of %2").arg(pageIndex + 1).arg(totalPages));
        }

        // Start new page if not the last
        if (pageIndex < totalPages - 1)
        {
            if (!printer->newPage())
                break;
        }
    }
}

} // anonymous namespace

// ============================================================================
// Public API
// ============================================================================

bool showPrintPreview(QWidget* parent, TextEditor* editor, const QString& documentDisplayName, bool lineNumbersVisible)
{
    if (!editor)
        return false;

    QPrinter printer(QPrinter::HighResolution);
    printer.setOutputFormat(QPrinter::NativeFormat);
    configurePrinter(&printer, documentDisplayName);

    QPrintPreviewDialog previewDialog(&printer, parent);
    previewDialog.setWindowTitle(QObject::tr("Print Preview"));

    QObject::connect(&previewDialog, &QPrintPreviewDialog::paintRequested, editor,
                     [editor, documentDisplayName, lineNumbersVisible](QPrinter* previewPrinter)
                     { renderDocument(editor, previewPrinter, documentDisplayName, lineNumbersVisible); });

    return previewDialog.exec() == QDialog::Accepted;
}

} // namespace GnotePad::ui::PrintSupport
