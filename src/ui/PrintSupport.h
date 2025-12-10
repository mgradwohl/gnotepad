#pragma once

#include <QtCore/qstring.h>

class QWidget;

namespace GnotePad::ui
{

class TextEditor;

namespace PrintSupport
{

/// Shows print preview dialog and handles printing.
/// @param parent Parent widget for dialogs
/// @param editor The text editor to print from
/// @param documentDisplayName Name shown in header and print job
/// @param lineNumbersVisible Whether to include line numbers in output
/// @return true if user accepted and printed, false if cancelled
bool showPrintPreview(QWidget* parent,
                      TextEditor* editor,
                      const QString& documentDisplayName,
                      bool lineNumbersVisible);

} // namespace PrintSupport

} // namespace GnotePad::ui
