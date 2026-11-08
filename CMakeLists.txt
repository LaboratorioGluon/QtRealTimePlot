cmake_minimum_required(VERSION 3.16)
project(RealtimePlot VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

option(ENABLE_CPPCHECK "Run Cppcheck in compilation" OFF)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Choose build type: Debug Release RelWithDebInfo MinSizeRel" FORCE)
endif()

# ============================================================
# Qt packages
# ============================================================

find_package(Qt6 COMPONENTS Core Widgets OpenGL OpenGLWidgets REQUIRED)
set(QT_LIBS Qt6::Core Qt6::Widgets Qt6::OpenGL Qt6::OpenGLWidgets)
message(STATUS "Using Qt6")


find_package(OpenGL REQUIRED)

find_program(CPPCHECK_PATH cppcheck)
find_program(CLANG_FORMAT_PATH clang-format)

if(CPPCHECK_PATH)

    if(ENABLE_CPPCHECK)
        message(STATUS "Cppcheck: Enabled")
        set(CMAKE_CXX_CPPCHECK "${CPPCHECK_PATH}" 
            "--enable=all" 
            "--suppress=missingInclude"
            "--suppress=missingIncludeSystem"
            "--inline-suppr"
        )
    else()
        message(STATUS "Cppcheck: Disabled (Use -DENABLE_CPPCHECK=ON to enable it)")
    endif()


    file(GLOB_RECURSE ALL_ANALYZE_FILES 
        "${CMAKE_CURRENT_SOURCE_DIR}/include/*.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/example/*.cpp"
    )

    # Definimos los argumentos limpiamente en una lista de CMake
    set(CPPCHECK_ARGS
        --enable=all
        --language=c++
        --library=qt
        --inline-suppr
        --suppress=unmatchedSuppression
        --suppress=missingInclude
        --suppress=missingIncludeSystem
        -I${CMAKE_CURRENT_SOURCE_DIR}/include
        -I${CMAKE_CURRENT_SOURCE_DIR}/src
    )

    # Creamos el target pasando la lista de argumentos expandida
    add_custom_target(cppcheck-all
        COMMAND ${CPPCHECK_PATH} 
            --project=${CMAKE_BINARY_DIR}/compile_commands.json
            --enable=all
            --library=qt
            --inline-suppr
            --suppress=unmatchedSuppression
            --suppress=missingInclude
            --suppress=missingIncludeSystem
            -I ${CMAKE_CURRENT_SOURCE_DIR}/include
        COMMENT "Ejecutando análisis de programa completo (Whole Program Analysis)..."
        VERBATIM
    )
else()
    message(WARNING "Cppcheck no encontrado. El análisis estático estará desactivado.")
endif()


if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    # Project-wide Qt defaults; leave them to the parent project when used as a subdirectory.
    qt_standard_project_setup()
endif()

# ============================================================
# Library target
# ============================================================
add_library(realtimeplot STATIC
    # Public API
    include/RealtimePlot.h
    include/SampleBuffer.h
    include/PlotSeries.h
    include/PlotView.h
    include/PlotAxis.h
    include/PlotOverlay.h
    include/PlotCursor.h
    include/PlotRangeCursor.h
    include/PlotLegend.h
    # Widget
    src/RealtimePlot.cpp
    src/RealtimePlotInput.cpp
    # Data and view
    src/SampleBuffer.cpp
    src/PlotView.cpp
    src/PlotAxis.cpp
    # Rendering (internal)
    src/PlotRenderer.h
    src/PlotRenderer.cpp
    src/Decimation.h
    src/Decimation.cpp
    # Overlays
    src/PlotCursor.cpp
    src/PlotRangeCursor.cpp
    src/PlotLegend.cpp
)

if(CPPCHECK_PATH AND ENABLE_CPPCHECK)
    set_target_properties(realtimeplot PROPERTIES
        CXX_CPPCHECK "${CPPCHECK_PATH};-I${CMAKE_CURRENT_SOURCE_DIR}/include;-I${CMAKE_CURRENT_SOURCE_DIR}/src;--enable=all;--library=qt;--inline-suppr;--suppress=unmatchedSuppression;--suppress=missingInclude;--suppress=missingIncludeSystem;--suppress=*:*_autogen*"
    )
endif()

if(CLANG_FORMAT_PATH)
    # Opt-in formatting: run `cmake --build <dir> --target realtimeplot-format`.
    # It is not part of the normal build so builds never modify the sources.
    get_target_property(LYB_SOURCES realtimeplot SOURCES)
    file(GLOB_RECURSE LYB_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}/include/*.h")
    add_custom_target(realtimeplot-format
        COMMAND ${CLANG_FORMAT_PATH} -i -style=file ${LYB_SOURCES} ${LYB_HEADERS}
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        COMMENT "Formatting realtimeplot sources with clang-format"
        VERBATIM
    )
endif()

target_compile_definitions(realtimeplot PRIVATE
    $<$<CONFIG:Release>:QT_NO_DEBUG_OUTPUT>
)

target_include_directories(realtimeplot
    PUBLIC  ${CMAKE_CURRENT_SOURCE_DIR}/include
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(realtimeplot
    PUBLIC ${QT_LIBS} OpenGL::GL
)

# ============================================================
# Example and tests (built by default only when this is the top-level project)
# ============================================================
if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    set(RTP_TOP_LEVEL ON)
else()
    set(RTP_TOP_LEVEL OFF)
endif()

option(RTP_BUILD_EXAMPLES "Build the realtimeplot demo application" ${RTP_TOP_LEVEL})
option(RTP_BUILD_TESTS "Build the realtimeplot unit tests" ${RTP_TOP_LEVEL})

if(RTP_BUILD_EXAMPLES)
    add_executable(realtimeplot_demo
        example/main.cpp
    )
    target_compile_definitions(realtimeplot_demo PRIVATE
        $<$<CONFIG:Release>:QT_NO_DEBUG_OUTPUT>
    )
    target_link_libraries(realtimeplot_demo
        PRIVATE realtimeplot ${QT_LIBS}
    )
endif()

if(RTP_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

# ============================================================
# Install rules (optional)
# ============================================================
install(TARGETS realtimeplot
        ARCHIVE DESTINATION lib
        LIBRARY DESTINATION lib)
install(DIRECTORY include/ DESTINATION include/realtimeplot)
