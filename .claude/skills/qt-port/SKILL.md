---
name: qt-port
description: Port a Qt5 component to Qt6. Pass the component directory path as args (e.g. /qt-port Src/Components/wac_network).
---

You are porting a component of the hallamp codebase from Qt 5.12 to Qt 6. The component path is: {{ args }}

## Steps

1. **Read all source files** in the component directory (`.cpp`, `.h`, `.pri`, `.pro` if present)

2. **Identify Qt5-specific usage** — check for:
   - `Qt5::` CMake targets (should become `Qt6::`)
   - `QWebEngineView` / `QWebEnginePage` API differences (script injection, settings API changed)
   - `QNetworkAccessManager` — largely compatible but check `QSslConfiguration` usage
   - `QRegExp` → `QRegularExpression` (QRegExp removed in Qt6)
   - `QStringRef` → `QStringView` or `QString` (QStringRef removed in Qt6)
   - `QLinkedList` → `std::list` (removed in Qt6)
   - `QVariant::Type` → `QMetaType` (type system refactored)
   - `QTextCodec` → `QStringConverter` (removed in Qt6)
   - `Qt::MiddleButton` / button enum changes
   - `QDesktopWidget` → `QScreen` (removed in Qt6)
   - `QApplication::desktop()` → `QGuiApplication::screens()`
   - `QFontMetrics::width()` → `QFontMetrics::horizontalAdvance()`
   - Implicit `QString` conversions (stricter in Qt6)
   - `#include <QtWidgets/QApplication>` vs module changes

3. **For each finding**, output:
   - The Qt5 API / pattern used (exact code)
   - The Qt6 replacement (exact corrected code)
   - Any behavior difference to be aware of
   - Whether it's a `drop-in` (same semantics) or `requires-testing` change

4. **CMakeLists.txt update**: Show the corrected `find_package` and `target_link_libraries` using Qt6 targets:
   ```cmake
   find_package(Qt6 REQUIRED COMPONENTS Core Network WebEngineWidgets)
   target_link_libraries(${TARGET} PRIVATE Qt6::Core Qt6::Network Qt6::WebEngineWidgets)
   ```

5. **Flag blockers**: Anything that requires a more significant architectural change (not a simple rename), especially `wac_browser`'s use of `QWebEnginePage` for custom script injection and the JS bridge API.

## Reference

- Qt6 porting guide: https://doc.qt.io/qt-6/sourcebreaks.html
- Qt6 CMake migration: https://doc.qt.io/qt-6/cmake-qt5-and-qt6-compatibility.html
- QWebEngine Qt6 changes: https://doc.qt.io/qt-6/whatsnew60.html
