cmake_minimum_required (VERSION 2.8.12)
project(OgreImgui)

# specify which version you need
find_package(OGRE 1.10 REQUIRED)
find_package(Freetype QUIET)

# the search paths
include_directories(${OGRE_INCLUDE_DIRS})
link_directories(${OGRE_LIBRARY_DIRS})

set(IMGUI_SRCS 
    ${CMAKE_SOURCE_DIR}/imgui/imgui.cpp 
    ${CMAKE_SOURCE_DIR}/imgui/imgui_draw.cpp
    ${CMAKE_SOURCE_DIR}/imgui/imgui_widgets.cpp)
set(OGRE_IMGUI_SRCS ${CMAKE_SOURCE_DIR}/ImguiManager.cpp ${CMAKE_SOURCE_DIR}/ImguiRenderable.cpp)
if(FREETYPE_FOUND)
    include_directories(${CMAKE_SOURCE_DIR}/imgui/misc/freetype/)
    list(APPEND IMGUI_SRCS ${CMAKE_SOURCE_DIR}/imgui/misc/freetype/imgui_freetype.cpp)
endif()

add_definitions(-DIMGUI_DISABLE_OBSOLETE_FUNCTIONS)
include_directories(${CMAKE_SOURCE_DIR} ${CMAKE_SOURCE_DIR}/imgui/)
add_library(OgreImgui ${IMGUI_SRCS} ${OGRE_IMGUI_SRCS})
set_property(TARGET OgreImgui PROPERTY POSITION_INDEPENDENT_CODE ON)
target_link_libraries(OgreImgui PUBLIC ${OGRE_LIBRARIES})

if(FREETYPE_FOUND)
    target_compile_definitions(OgreImgui PRIVATE -DUSE_FREETYPE)
    target_include_directories(OgreImgui PRIVATE ${FREETYPE_INCLUDE_DIRS})
    target_link_libraries(OgreImgui PRIVATE ${FREETYPE_LIBRARIES})
endif()

option(BUILD_EXAMPLE "build example app" ON)
if(BUILD_EXAMPLE)
    # copy essential config files next to our binary where OGRE autodiscovers them
    file(COPY ${OGRE_CONFIG_DIR}/resources.cfg DESTINATION ${CMAKE_BINARY_DIR})

    add_executable(Example Example.cpp ${CMAKE_SOURCE_DIR}/imgui/imgui_demo.cpp)
    target_link_libraries(Example OgreImgui)

    file(COPY ${OGRE_CONFIG_DIR}/resources.cfg DESTINATION ${CMAKE_SOURCE_DIR}/python/)
endif()

if(NOT FREETYPE_FOUND)
    set(FREETYPE_FOUND "FALSE")
endif()
message(STATUS "using freetype: ${FREETYPE_FOUND}")

# Python Bindings
option(BUILD_PYTHON "build python bindings" ON)
if(BUILD_PYTHON)
    add_subdirectory(python)
endif()