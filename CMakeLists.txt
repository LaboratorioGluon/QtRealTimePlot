cmake_minimum_required(VERSION 3.16)
project(RealtimePlot VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

# ============================================================
# Qt packages
# ============================================================

find_package(Qt6 COMPONENTS Core Widgets OpenGL OpenGLWidgets REQUIRED)
set(QT_LIBS Qt6::Core Qt6::Widgets Qt6::OpenGL Qt6::OpenGLWidgets)
message(STATUS "Using Qt6")


find_package(OpenGL REQUIRED)



# ============================================================
# Library target
# ============================================================
add_library(realtimeplot STATIC
    include/RealtimePlot.h
    src/PlotSeries.cpp
    src/PlotAxis.cpp
    src/RealtimePlot_init.cpp
    src/RealtimePlot_paint2.cpp
    src/RealtimePlot_input.cpp
    src/components/RtpLegend.cpp
)

target_include_directories(realtimeplot
    PUBLIC  ${CMAKE_CURRENT_SOURCE_DIR}/include
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(realtimeplot
    PUBLIC ${QT_LIBS} OpenGL::GL
)

# ============================================================
# Example executable
# ============================================================

qt_standard_project_setup()

add_executable(realtimeplot_demo
    example/main.cpp
)


target_link_libraries(realtimeplot_demo
    PRIVATE realtimeplot ${QT_LIBS}
)

# ============================================================
# Install rules (optional)
# ============================================================
install(TARGETS realtimeplot
        ARCHIVE DESTINATION lib
        LIBRARY DESTINATION lib)
install(DIRECTORY include/ DESTINATION include/realtimeplot)
