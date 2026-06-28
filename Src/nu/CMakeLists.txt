# nu (Nullsoft Utility lib) — portable subset only.
#
# This intentionally builds just the cross-platform data structures, buffers,
# and codecs that compile cleanly on Linux. Deferred (each is its own bite):
#   - Win32/GUI files (HTMLContainer*, windowsTheme, mtbrowser, menuHelpers,
#     listview, comboskin, ChildSizer, DialogSkinner) — Phase 4 GUI port.
#     DialogSkinner looks tiny but its header pulls MediaLibraryInterface.h ->
#     <windows.h>.
#   - RedBlackTree.cpp — depends on bfc::PtrList (<bfc/ptrlist.h>), which drags
#     in the full Wasabi container stack; port that separately.
#   - MSVC CRT-shrinking stubs (noarg/nochkclr/noenv/nomsgs/nosec) — Windows
#     linker tricks, meaningless on glibc.
#   - nonewthrow.c — a *global* operator new/delete override; must not be
#     silently linked into every nu consumer (would break Qt / C++ OOM).
#
# strsafe.c + trace.cpp now build: strsafe.h's wide printf path was ported to
# Linux (it delegates to native-wchar_t vswprintf; see strsafe.h). NOTE the wide
# path operates on the platform-native 32-bit wchar_t, NOT the codebase's 16-bit
# WCHAR — reconciling the WCHAR* StringCch*W callers is a separate deferred bite.
add_library(nu STATIC
    bitbuffer.cpp
    RingBuffer.cpp
    GaplessRingBuffer.cpp
    SpillBuffer.cpp
    sort.cpp
    regexp.cpp
    ThreadQueue.cpp
    ServiceWatcher.cpp
    strsafe.c
    trace.cpp
)

# nu headers reference <bfc/...>, <api/...> (Wasabi) and foundation/ (replicant).
# Do NOT add Src/pfc to the path (its string.h shadows the standard <string.h>).
# Src/nu itself is on the path so trace.cpp's <strsafe.h> resolves (no Src/nu/string.h).
target_include_directories(nu PUBLIC ${CMAKE_SOURCE_DIR}/Src)
target_include_directories(nu PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/Src/Wasabi
    ${CMAKE_SOURCE_DIR}/Src/replicant
)

# --- strsafe runtime proof (the inline path trace.cpp depends on) ---
# Compiled with _DEBUG so trace.cpp's bodies exist; carries its own OutputDebugString
# sink so it links without the (not-yet-built) Wasabi linux.cpp.
add_executable(nu_strsafe_test
    test/strsafe_test.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/trace.cpp
)
target_compile_definitions(nu_strsafe_test PRIVATE _DEBUG)
target_include_directories(nu_strsafe_test PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/Src
)
add_test(NAME nu_strsafe COMMAND nu_strsafe_test)
