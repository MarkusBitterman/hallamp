add_library(pfc STATIC
    grow_buf.cpp
    # cfg_var.cpp and string*.cpp need <windows.h> fixes — added in later phases
)

target_include_directories(pfc PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
