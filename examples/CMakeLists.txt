# sources for examples
set(SOURCES contexts delta_schunk_ex multithread simple simple_schunk frame_simple
        frame_backed_schunk frame_metalayers compress_file noinit find_roots zstd_dict)
if (EXISTS btune_schunk.c)
    set(SOURCES ${SOURCES} btune_schunk.c)
endif()

# targets
foreach(src ${SOURCES})
    add_executable(${src} ${src}.c)
    # have to copy blosc dlls on Windows
    if (MSVC)
        add_custom_command(
                TARGET ${src}
                POST_BUILD
                COMMAND ${CMAKE_COMMAND}
                ARGS -E copy_if_different
                "${PROJECT_BINARY_DIR}/blosc/\$\(Configuration\)/blosc.dll"
                "${CMAKE_CURRENT_BINARY_DIR}/\$\(Configuration\)/blosc.dll")
    elseif (MINGW)
        add_custom_command(
                TARGET ${src}
                POST_BUILD
                COMMAND ${CMAKE_COMMAND}
                ARGS -E copy_if_different
                "${PROJECT_BINARY_DIR}/blosc/libblosc.dll"
                "${CMAKE_CURRENT_BINARY_DIR}/libblosc.dll")
    endif(MSVC)
    # Define the BLOSC_TESTING symbol so normally-hidden functions
    # aren't hidden from the view of the test programs.
    set_property(
            TARGET ${src}
            APPEND PROPERTY COMPILE_DEFINITIONS BLOSC_TESTING)
endforeach(src)

if (UNIX AND NOT APPLE)
    # cmake is complaining about LINK_PRIVATE in original PR
    # and removing it does not seem to hurt, so be it.
    # target_link_libraries(bench LINK_PRIVATE rt)
    foreach(src ${SOURCES})
        target_link_libraries(${src} rt)
    endforeach(src)
endif (UNIX AND NOT APPLE)

foreach(src ${SOURCES})
    target_link_libraries(${src} blosc_shared)
endforeach(src)

# tests
if (BUILD_TESTS)
    option(TEST_INCLUDE_EXAMPLES "Include examples in the tests" ON)
    if (TEST_INCLUDE_EXAMPLES)
        foreach(src ${SOURCES})
            if(src STREQUAL compress_file)
                add_test(test_example_${src} ${src} "${PROJECT_BINARY_DIR}/CMakeCache.txt" CMakeCache.b2frame)
            else()
                add_test(test_example_${src} ${src})
            endif(src STREQUAL compress_file)
        endforeach(src)
    endif()
endif (BUILD_TESTS)
