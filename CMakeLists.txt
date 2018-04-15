cmake_minimum_required (VERSION 2.8)
project(OgreImgui)

# specify which version you need
find_package(OGRE 1.10 REQUIRED)

# the search paths
include_directories(${OGRE_INCLUDE_DIRS})
link_directories(${OGRE_LIBRARY_DIRS})

# copy essential config files next to our binary where OGRE autodiscovers them
file(COPY ${OGRE_CONFIG_DIR}/plugins.cfg ${OGRE_CONFIG_DIR}/resources.cfg
     DESTINATION ${CMAKE_BINARY_DIR})

set(IMGUI_SRCS ${CMAKE_SOURCE_DIR}/imgui/imgui.cpp ${CMAKE_SOURCE_DIR}/imgui/imgui_draw.cpp)
set(OGRE_IMGUI_SRCS ${CMAKE_SOURCE_DIR}/ImguiManager.cpp ${CMAKE_SOURCE_DIR}/ImguiRenderable.cpp)

include_directories(${CMAKE_SOURCE_DIR} ${CMAKE_SOURCE_DIR}/imgui/)
add_library(OgreImgui ${IMGUI_SRCS} ${OGRE_IMGUI_SRCS})
target_link_libraries(OgreImgui ${OGRE_LIBRARIES})

add_executable(Example Example.cpp ${CMAKE_SOURCE_DIR}/imgui/imgui_demo.cpp)
target_link_libraries(Example OgreImgui)
