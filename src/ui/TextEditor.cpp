#include "ui/TextEditor.h"

#include <QApplication>
#include <QList>
#include <QPaintEvent>
#include <QPainter>
#include <QScrollBar>
#include <QTextBlock>
#include <QTextEdit>
#include <QTextFormat>
#include <QWheelEvent>

#include <algorithm>

namespace GnotePad::ui
{

namespace
{
constexpr int kZoomStepPercent = 10;
constexpr int kMinZoomPercent = 10;
constexpr int kMaxZoomPercent = 500;
}

TextEditor::LineNumberArea::LineNumberArea(TextEditor* editor)
    : QWidget(editor)
    , m_editor(editor)
{
    setCursor(Qt::ArrowCursor);
}

QSize TextEditor::LineNumberArea::sizeHint() const
{
    if(!m_editor)
    {
        return QSize(0, 0);
    }
    return QSize(m_editor->lineNumberAreaWidth(), 0);
}

void TextEditor::LineNumberArea::paintEvent(QPaintEvent* event)
{
    if(m_editor)
    {
        m_editor->lineNumberAreaPaintEvent(event);
    }
}

TextEditor::TextEditor(QWidget* parent)
    : QPlainTextEdit(parent)
    , m_lineNumberArea(new LineNumberArea(this))
    , m_defaultFont(font())
{
    m_lineNumberArea->setVisible(m_lineNumbersVisible);

    connect(this, &QPlainTextEdit::blockCountChanged, this, &TextEditor::updateLineNumberAreaWidth);
    connect(this, &QPlainTextEdit::updateRequest, this, &TextEditor::updateLineNumberArea);
    connect(this, &QPlainTextEdit::cursorPositionChanged, this, &TextEditor::highlightCurrentLine);

    updateLineNumberAreaWidth(0);
    highlightCurrentLine();
    updateTabStopDistance();
}

void TextEditor::setLineNumbersVisible(bool visible)
{
    if(m_lineNumbersVisible == visible)
    {
        return;
    }

    m_lineNumbersVisible = visible;
    if(m_lineNumberArea)
    {
        m_lineNumberArea->setVisible(m_lineNumbersVisible);
    }
    updateLineNumberAreaWidth(0);
}

int TextEditor::lineNumberAreaWidth() const
{
    if(!m_lineNumbersVisible)
    {
        return 0;
    }

    int digits = 1;
    int max = std::max(1, blockCount());
    while(max >= 10)
    {
        max /= 10;
        ++digits;
    }

    const int space = 8 + fontMetrics().horizontalAdvance(QLatin1Char('9')) * digits;
    return space;
}

void TextEditor::lineNumberAreaPaintEvent(QPaintEvent* event)
{
    if(!m_lineNumberArea || !m_lineNumbersVisible)
    {
        return;
    }

    QPainter painter(m_lineNumberArea);
    painter.fillRect(event->rect(), palette().alternateBase());

    QTextBlock block = firstVisibleBlock();
    int blockNumber = block.blockNumber();
    int top = static_cast<int>(blockBoundingGeometry(block).translated(contentOffset()).top());
    int bottom = top + static_cast<int>(blockBoundingRect(block).height());

    const QColor inactiveColor = palette().color(QPalette::Disabled, QPalette::Text);
    const QColor activeColor = palette().color(QPalette::Text);
    const int currentBlockNumber = textCursor().blockNumber();

    while(block.isValid() && top <= event->rect().bottom())
    {
        if(block.isVisible() && bottom >= event->rect().top())
        {
            const QString number = QString::number(blockNumber + 1);
            painter.setPen(blockNumber == currentBlockNumber ? activeColor : inactiveColor);
            painter.drawText(0, top, m_lineNumberArea->width() - 6, fontMetrics().height(), Qt::AlignRight, number);
        }

        block = block.next();
        top = bottom;
        bottom = top + static_cast<int>(blockBoundingRect(block).height());
        ++blockNumber;
    }
}

void TextEditor::updateLineNumberAreaWidth(int)
{
    setViewportMargins(lineNumberAreaWidth(), 0, 0, 0);
}

void TextEditor::updateLineNumberArea(const QRect& rect, int dy)
{
    if(!m_lineNumberArea)
    {
        return;
    }

    if(dy != 0)
    {
        m_lineNumberArea->scroll(0, dy);
    }
    else
    {
        m_lineNumberArea->update(0, rect.y(), m_lineNumberArea->width(), rect.height());
    }

    if(rect.contains(viewport()->rect()))
    {
        updateLineNumberAreaWidth(0);
    }
}

void TextEditor::resizeEvent(QResizeEvent* event)
{
    QPlainTextEdit::resizeEvent(event);

    if(!m_lineNumberArea)
    {
        return;
    }

    const QRect cr = contentsRect();
    const int width = lineNumberAreaWidth();
    m_lineNumberArea->setGeometry(QRect(cr.left(), cr.top(), width, cr.height()));
}

void TextEditor::highlightCurrentLine()
{
    if(isReadOnly())
    {
        return;
    }

    QTextEdit::ExtraSelection selection;
    selection.format.setBackground(palette().alternateBase());
    selection.format.setProperty(QTextFormat::FullWidthSelection, true);
    selection.cursor = textCursor();
    selection.cursor.clearSelection();

    QList<QTextEdit::ExtraSelection> selections;
    selections << selection;
    setExtraSelections(selections);
}

void TextEditor::increaseZoom(int range)
{
    // Check if we would exceed the maximum zoom percentage
    const int newPercentage = m_zoomPercentage + range * kZoomStepPercent;
    if(newPercentage > kMaxZoomPercent)
    {
        return; // Already at maximum zoom
    }
    
    QPlainTextEdit::zoomIn(range);
    updateZoomPercentageEstimate(range);
    updateTabStopDistance();
}

void TextEditor::decreaseZoom(int range)
{
    // Check if we would go below the minimum zoom percentage
    const int newPercentage = m_zoomPercentage - range * kZoomStepPercent;
    if(newPercentage < kMinZoomPercent)
    {
        return; // Already at minimum zoom
    }
    
    QPlainTextEdit::zoomOut(range);
    updateZoomPercentageEstimate(-range);
    updateTabStopDistance();
}

void TextEditor::wheelEvent(QWheelEvent* event)
{
    if(event->modifiers().testFlag(Qt::ControlModifier))
    {
        event->accept();
        if(event->angleDelta().y() > 0)
        {
            increaseZoom();
        }
        else if(event->angleDelta().y() < 0)
        {
            decreaseZoom();
        }
        return;
    }

    QPlainTextEdit::wheelEvent(event);
}

void TextEditor::resetZoom()
{
    QPlainTextEdit::setFont(m_defaultFont);
    m_zoomPercentage = 100;
    emit zoomPercentageChanged(m_zoomPercentage);
    updateLineNumberAreaWidth(0);
    updateTabStopDistance();
}

void TextEditor::applyEditorFont(const QFont& font)
{
    m_defaultFont = font;
    QPlainTextEdit::setFont(font);
    m_zoomPercentage = 100;
    emit zoomPercentageChanged(m_zoomPercentage);
    updateLineNumberAreaWidth(0);
    updateTabStopDistance();
}

void TextEditor::setZoomPercentage(int percent)
{
    const int clamped = std::clamp(percent, kMinZoomPercent, kMaxZoomPercent);
    const int snapped = (clamped / kZoomStepPercent) * kZoomStepPercent;
    const int delta = snapped - m_zoomPercentage;
    if(delta == 0)
    {
        return;
    }

    if(delta > 0)
    {
        increaseZoom(delta / kZoomStepPercent);
    }
    else
    {
        decreaseZoom((-delta) / kZoomStepPercent);
    }
}

void TextEditor::updateTabStopDistance()
{
    const QFontMetricsF metrics(font());
    setTabStopDistance(std::max(1, m_tabSizeSpaces) * metrics.horizontalAdvance(QStringLiteral(" ")));
}

void TextEditor::updateZoomPercentageEstimate(int deltaSteps)
{
    m_zoomPercentage = std::clamp(m_zoomPercentage + deltaSteps * kZoomStepPercent, kMinZoomPercent, kMaxZoomPercent);
    emit zoomPercentageChanged(m_zoomPercentage);
}

void TextEditor::setTabSizeSpaces(int spaces)
{
    const int normalized = std::clamp(spaces, 1, 16);
    if(m_tabSizeSpaces == normalized)
    {
        return;
    }
    m_tabSizeSpaces = normalized;
    updateTabStopDistance();
}

} // namespace GnotePad::ui
