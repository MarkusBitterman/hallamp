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
#   - trace.cpp + strsafe.c — depend on a strsafe.h that still uses __declspec;
#     needs a strsafe.h port first.
#   - MSVC CRT-shrinking stubs (noarg/nochkclr/noenv/nomsgs/nosec) — Windows
#     linker tricks, meaningless on glibc.
#   - nonewthrow.c — a *global* operator new/delete override; must not be
#     silently linked into every nu consumer (would break Qt / C++ OOM).
add_library(nu STATIC
    bitbuffer.cpp
    RingBuffer.cpp
    GaplessRingBuffer.cpp
    SpillBuffer.cpp
    sort.cpp
    regexp.cpp
    ThreadQueue.cpp
    ServiceWatcher.cpp
)

# nu headers reference <bfc/...>, <api/...> (Wasabi) and foundation/ (replicant).
# Do NOT add Src/pfc to the path (its string.h shadows the standard <string.h>).
target_include_directories(nu PUBLIC ${CMAKE_SOURCE_DIR}/Src)
target_include_directories(nu PRIVATE
    ${CMAKE_SOURCE_DIR}/Src/Wasabi
    ${CMAKE_SOURCE_DIR}/Src/replicant
)
