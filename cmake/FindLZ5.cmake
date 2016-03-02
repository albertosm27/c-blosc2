find_path(LZ5_INCLUDE_DIR lz5.h)

find_library(LZ5_LIBRARY NAMES lz5)

if (LZ5_INCLUDE_DIR AND LZ5_LIBRARY)
    set(LZ5_FOUND TRUE)
    message(STATUS "Found LZ5 library: ${LZ5_LIBRARY}")
else ()
    message(STATUS "No LZ5 library found.  Using internal sources.")
endif ()
