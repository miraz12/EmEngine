ADD_LIBRARY(exts INTERFACE)

if("${CMAKE_SYSTEM}" MATCHES "Linux" OR "${CMAKE_SYSTEM}" MATCHES "Darwin-*")

    find_package(OpenGL REQUIRED)

    add_library(glad STATIC glad/src/glad.c glad/include/glad/glad.h)
    # GLAD
    TARGET_INCLUDE_DIRECTORIES(exts SYSTEM INTERFACE glad/include)



    set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
    set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
    set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
    add_subdirectory(glfw)
endif()


# GLFW
TARGET_INCLUDE_DIRECTORIES(exts SYSTEM INTERFACE glfw/include)


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

# STB_IMAGE
TARGET_INCLUDE_DIRECTORIES(exts SYSTEM INTERFACE stb_image)

TARGET_LINK_LIBRARIES(exts INTERFACE glfw ${OPENGL_LIBRARIES} glm imgui)
