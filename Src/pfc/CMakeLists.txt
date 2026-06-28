add_library(pfc STATIC
    grow_buf.cpp
    cfg_var.cpp
    string.cpp
    string_unicode.cpp
)

# IMPORTANT: do NOT add Src/pfc to the include path. This directory contains a
# header literally named string.h, which would shadow the C/C++ standard
# <string.h>/<cstring> for every translation unit that sees the -I. pfc's own
# sources resolve sibling headers via quote-include directory search instead,
# and consumers include via "pfc/...".
target_include_directories(pfc PUBLIC ${CMAKE_SOURCE_DIR}/Src)

# Win32 type shims (WCHAR/HKEY/HWND/BYTE/UINT) come from <windows.h> on Windows
# and from the Wasabi platform layer (bfc/platform/platform.h) on Linux.
target_include_directories(pfc PRIVATE ${CMAKE_SOURCE_DIR}/Src/Wasabi)
