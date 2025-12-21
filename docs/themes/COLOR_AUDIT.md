# GnotePad Color Usage Audit

This document catalogs all color usages in the GnotePad codebase and the plan to migrate to a theme-based system.

## Current Color Usage

### 1. Direct Hardcoded Colors (Need Refactoring)

#### PrintSupport.cpp
Location: `src/ui/PrintSupport.cpp`

These colors are used for printing to ensure proper black-on-white output for print/PDF:

1. **Line 49**: `fmt.setForeground(Qt::black);`
   - Context: Setting text color for printed document
   - Used in: `forcePrintColors()` function
   - Purpose: Ensure text is black for printing
   
2. **Line 232**: `painter.setBrush(Qt::white);`
   - Context: Drawing page background
   - Used in: `renderDocument()` function (page rendering loop)
   - Purpose: White background for each page
   
3. **Line 234**: `painter.setBackground(Qt::white);`
   - Context: Setting painter background mode
   - Used in: `renderDocument()` function
   - Purpose: Ensure background is white
   
4. **Line 238**: `painter.setPen(Qt::black);`
   - Context: Drawing text with black pen
   - Used in: `renderDocument()` function
   - Purpose: Black text for readability on white background
   
5. **Line 302**: `painter.setBackground(Qt::white);`
   - Context: Setting background for page content drawing
   - Used in: `renderDocument()` function (content section)
   - Purpose: White background for document content
   
6. **Line 303**: `painter.setPen(Qt::black);`
   - Context: Drawing page content text
   - Used in: `renderDocument()` function (content section)
   - Purpose: Black text for page content

**Analysis**: These are intentionally hardcoded for print output. However, we should consider:
- Adding theme support for print colors (some users prefer dark mode printing)
- Making print colors configurable via theme
- Default should remain black-on-white for compatibility

### 2. Theme-Aware Palette Colors (Already Correct)

#### TextEditor.cpp
Location: `src/ui/TextEditor.cpp`

These already use Qt's palette system and will automatically adapt to themes:

1. **Line 113**: `painter.fillRect(event->rect(), palette().alternateBase());`
   - Context: Line number area background
   - Used in: `lineNumberAreaPaintEvent()`
   - Status: ✅ Already theme-aware
   
2. **Line 123**: `const QColor inactiveColor = palette().color(QPalette::Disabled, QPalette::Text);`
   - Context: Inactive line numbers color
   - Used in: `lineNumberAreaPaintEvent()`
   - Status: ✅ Already theme-aware
   
3. **Line 124**: `const QColor activeColor = palette().color(QPalette::Text);`
   - Context: Active line number color (current line)
   - Used in: `lineNumberAreaPaintEvent()`
   - Status: ✅ Already theme-aware
   
4. **Line 193**: `selection.format.setBackground(palette().alternateBase());`
   - Context: Current line highlight background
   - Used in: `highlightCurrentLine()`
   - Status: ✅ Already theme-aware

## Refactoring Plan

### Phase 1: Create Theme Infrastructure

1. **Create Theme TOML Schema**
   - Define color properties for all UI elements
   - Include print colors as optional override
   - Support QPalette color roles

2. **Implement Theme Loader**
   - Parse TOML files
   - Validate color values
   - Apply to QPalette

3. **Integrate with Application**
   - Load theme on startup
   - Allow runtime theme switching
   - Persist theme preference in settings

### Phase 2: Refactor PrintSupport.cpp

Current approach:
```cpp
painter.setPen(Qt::black);
painter.setBackground(Qt::white);
```

Proposed theme-aware approach:
```cpp
// Get print colors from theme, with fallback to black/white
const QColor printTextColor = themeManager.getPrintTextColor();    // default: Qt::black
const QColor printBgColor = themeManager.getPrintBackgroundColor(); // default: Qt::white

painter.setPen(printTextColor);
painter.setBackground(printBgColor);
```

### Phase 3: No Changes Needed for TextEditor.cpp

TextEditor.cpp already uses `palette()` correctly and will automatically pick up theme changes when QPalette is updated.

## Theme TOML Structure (Proposed)

```toml
[theme]
name = "Windows Light"
version = "1.0"
author = "GnotePad Team"
description = "Light theme matching Windows 11 default appearance"

[colors.palette]
# QPalette color roles
window = "#F3F3F3"
window_text = "#000000"
base = "#FFFFFF"
alternate_base = "#F7F7F7"
text = "#000000"
button = "#CCCCCC"
button_text = "#000000"
highlight = "#0078D4"
highlight_text = "#FFFFFF"

[colors.print]
# Optional: Override colors for printing
text = "#000000"
background = "#FFFFFF"

[colors.custom]
# Custom application-specific colors if needed
# Currently none required
```

## Migration Checklist

- [ ] Create theme TOML schema
- [ ] Implement TOML parser (use existing C++ TOML library)
- [ ] Create ThemeManager class
- [ ] Update Application.cpp to load themes
- [ ] Refactor PrintSupport.cpp to use theme colors
- [ ] Create default theme files
- [ ] Create Windows 11 theme files
- [ ] Create Ubuntu 24.04 theme files
- [ ] Add theme selection UI
- [ ] Add theme preference persistence
- [ ] Update tests
- [ ] Update documentation

## Color Manipulation Instances

**Current Status**: No color manipulation found (no `.lighter()`, `.darker()`, `.setAlpha()`, RGB component access, etc.)

This is excellent - the codebase is clean and ready for theme integration!
