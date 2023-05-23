ADD_LIBRARY(exts INTERFACE)

if (NOT DEFINED EMSCRIPTEN)

    find_package(OpenGL REQUIRED)

    set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
    set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
    set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
    add_subdirectory(glfw)
endif()

# GLAD
add_library(glad STATIC glad/src/glad.c glad/include/glad/glad.h)
TARGET_INCLUDE_DIRECTORIES(glad PUBLIC glad/include)


# GLFW
TARGET_INCLUDE_DIRECTORIES(exts SYSTEM INTERFACE glfw/include)

# Bullet3
set(BUILD_CPU_DEMOS OFF)
set(BUILD_PYBULLE OFF)
set(BUILD_BULLET3 OFF)
ADD_SUBDIRECTORY(bullet3)
target_include_directories(exts SYSTEM INTERFACE bullet3/src)

# GLM
ADD_SUBDIRECTORY(glm)
TARGET_INCLUDE_DIRECTORIES(exts SYSTEM INTERFACE glm/include)

# IMGUI
ADD_LIBRARY(imgui STATIC  imgui/imgui.cpp 
                          imgui/imgui_demo.cpp
                          imgui/imgui_draw.cpp
                          imgui/imgui_internal.h
                          imgui/imgui_tables.cpp
                          imgui/imgui_widgets.cpp
                          imgui/imstb_rectpack.h
                          imgui/imstb_textedit.h
                          imgui/imstb_truetype.h
                          imgui/backends/imgui_impl_glfw.h
                          imgui/backends/imgui_impl_glfw.cpp
                          imgui/backends/imgui_impl_opengl3.h
                          imgui/backends/imgui_impl_opengl3.cpp)
TARGET_INCLUDE_DIRECTORIES(imgui PUBLIC imgui glfw/include)

# Header only
TARGET_INCLUDE_DIRECTORIES(exts SYSTEM INTERFACE header_only)


TARGET_LINK_LIBRARIES(exts INTERFACE glfw ${OPENGL_LIBRARIES} glm imgui BulletDynamics BulletCollision LinearMath)

if (NOT DEFINED EMSCRIPTEN)
TARGET_LINK_LIBRARIES(exts INTERFACE glad)
endif()
