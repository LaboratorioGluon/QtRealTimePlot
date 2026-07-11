cmake_minimum_required(VERSION 3.16)
project(RealtimePlot VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

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

find_program(CPPCHECK_PATH cppcheck)
find_program(CLANG_FORMAT_PATH clang-format)

if(CPPCHECK_PATH)
    set(CMAKE_CXX_CPPCHECK "${CPPCHECK_PATH}" 
        "--enable=all" 
        "--suppress=missingInclude"
        "--suppress=missingIncludeSystem"
        "--inline-suppr"
    )

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
    src/components/RtpCursor.cpp
    src/components/RtpRangeCursor.cpp
)

if(CPPCHECK_PATH)
    set_target_properties(realtimeplot PROPERTIES
        CXX_CPPCHECK "${CPPCHECK_PATH};-I${CMAKE_CURRENT_SOURCE_DIR}/include;-I${CMAKE_CURRENT_SOURCE_DIR}/src;--enable=all;--library=qt;--inline-suppr;--suppress=unmatchedSuppression;--suppress=missingInclude;--suppress=missingIncludeSystem;--suppress=*:*_autogen*"
    )
else()
    message(WARNING "Cppcheck no encontrado. El análisis estático estará desactivado.")
endif()

if(CLANG_FORMAT_PATH)
    # Recuperamos las propiedades de los archivos fuentes del target
    get_target_property(LYB_SOURCES realtimeplot SOURCES)
    
    # Añadimos un comando que se ejecuta antes de compilar el target
    add_custom_command(
        TARGET realtimeplot PRE_BUILD
        COMMAND ${CLANG_FORMAT_PATH} -i -style=file ${LYB_SOURCES}
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        COMMENT "Formateando archivos de la librería realtimeplot antes de compilar..."
    )
endif()

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
