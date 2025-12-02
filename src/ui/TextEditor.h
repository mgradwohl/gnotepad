#pragma once

#include <QPlainTextEdit>

class QPaintEvent;
class QResizeEvent;
class QWheelEvent;

namespace GnotePad::ui
{

class TextEditor : public QPlainTextEdit
{
    Q_OBJECT

public:
    explicit TextEditor(QWidget* parent = nullptr);

    void setLineNumbersVisible(bool visible);
    bool lineNumbersVisible() const { return m_lineNumbersVisible; }

    void resetZoom();
    void applyEditorFont(const QFont& font);
    void increaseZoom(int range = 1);
    void decreaseZoom(int range = 1);

signals:
    void zoomPercentageChanged(int percentage);

protected:
    void resizeEvent(QResizeEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;

public:
    int lineNumberAreaWidth() const;
    void lineNumberAreaPaintEvent(QPaintEvent* event);

private slots:
    void updateLineNumberAreaWidth(int newBlockCount = 0);
    void updateLineNumberArea(const QRect& rect, int dy);
    void highlightCurrentLine();

private:
    class LineNumberArea;

    void updateZoomPercentageEstimate(int deltaSteps);

    QWidget* m_lineNumberArea {nullptr};
    bool m_lineNumbersVisible {true};
    QFont m_defaultFont;
    int m_zoomPercentage {100};
};

class TextEditor::LineNumberArea : public QWidget
{
public:
    explicit LineNumberArea(TextEditor* editor);

    QSize sizeHint() const override;

protected:
    void paintEvent(QPaintEvent* event) override;

private:
    TextEditor* m_editor {nullptr};
};

} // namespace GnotePad::ui
