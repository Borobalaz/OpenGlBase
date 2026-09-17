cmake_minimum_required(VERSION 3.20)

if(NOT DEFINED CMAKE_TOOLCHAIN_FILE)
    set(CMAKE_TOOLCHAIN_FILE
        "C:/vcpkg/scripts/buildsystems/vcpkg.cmake" CACHE STRING "")
endif()

project(Connectomics-Imaging)
set(CMAKE_CXX_STANDARD 17)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

find_package(OpenGL REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(glad CONFIG REQUIRED)
find_package(assimp CONFIG REQUIRED)
find_package(Qt6 REQUIRED COMPONENTS Core Gui OpenGL OpenGLWidgets Widgets)

find_path(STB_INCLUDE_DIR
    NAMES stb_image.h
    REQUIRED
)

file(GLOB_RECURSE ENGINE_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/engine/src/*.cpp"
)

file(GLOB_RECURSE UI_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/ui/src/widgets/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/ui/src/windows/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/ui/src/controllers/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/ui/src/qt-adapters/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/ui/src/styles/*.cpp"
)

add_library(engine SHARED ${ENGINE_SOURCES})
set_target_properties(engine PROPERTIES WINDOWS_EXPORT_ALL_SYMBOLS ON)

target_include_directories(engine PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/engine/include
    ${CMAKE_CURRENT_SOURCE_DIR}/engine/include/engine
    ${STB_INCLUDE_DIR}
)

target_link_libraries(engine PUBLIC
    OpenGL::GL
    glm::glm
    glad::glad
    assimp::assimp
)

add_executable(app_qt
    ${UI_SOURCES}
    ${CMAKE_CURRENT_SOURCE_DIR}/ui/src/app/main.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ui/include/ui/controllers/MainWindowShortcuts.h
    ${CMAKE_CURRENT_SOURCE_DIR}/ui/include/ui/widgets/OpenGLViewportWidget.h
    ${CMAKE_CURRENT_SOURCE_DIR}/ui/include/ui/widgets/InspectProviderWidget.h
    ${CMAKE_CURRENT_SOURCE_DIR}/ui/include/ui/widgets/InspectorWidget.h
    ${CMAKE_CURRENT_SOURCE_DIR}/ui/include/ui/state/RenderStatistics.h
    ${CMAKE_CURRENT_SOURCE_DIR}/ui/include/ui/widgets/RenderStatisticsWidget.h
    ${CMAKE_CURRENT_SOURCE_DIR}/ui/include/ui/widgets/SceneObjectListWidget.h
    ${CMAKE_CURRENT_SOURCE_DIR}/ui/include/ui/widgets/inspect_fields/InspectNumberFieldWidget.h
    ${CMAKE_CURRENT_SOURCE_DIR}/ui/include/ui/windows/WidgetsMainWindow.h
    ${CMAKE_CURRENT_SOURCE_DIR}/ui/include/ui/qt-adapters/QTSceneInspector.h
)

target_include_directories(app_qt PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/engine/include
    ${CMAKE_CURRENT_SOURCE_DIR}/engine/include/engine
    ${CMAKE_CURRENT_SOURCE_DIR}/ui/include
)

target_link_libraries(app_qt PRIVATE
    engine
    Qt6::Core
    Qt6::Gui
    Qt6::OpenGL
    Qt6::OpenGLWidgets
    Qt6::Widgets
)

if (ITK_FOUND)
    target_link_libraries(app_qt PRIVATE ${ITK_LIBRARIES})
    target_compile_definitions(app_qt PRIVATE CONNECTOMICS_ENABLE_ITK_IO=1)
endif()

# Copy shader files to build output directory (same location as executable)
add_custom_command(TARGET app_qt POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${CMAKE_CURRENT_SOURCE_DIR}/engine/shaders"
            "$<TARGET_FILE_DIR:app_qt>/shaders"
    COMMENT "Copying shaders to build output..."
)

# Also copy assets (models, textures, volumes)
add_custom_command(TARGET app_qt POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${CMAKE_CURRENT_SOURCE_DIR}/engine/assets"
            "$<TARGET_FILE_DIR:app_qt>/assets"
    COMMENT "Copying assets to build output..."
)