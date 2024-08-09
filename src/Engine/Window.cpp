#include "Window.hpp"
// #define _DEBUG_ 1

#ifndef EMSCRIPTEN
void GLAPIENTRY MessageCallback(GLenum /* source */, GLenum type,
                                GLuint /* id */, GLenum severity,
                                GLsizei /* length */, const GLchar *message,
                                const void * /* userParam */) {
  std::string msg("[OPENGL DEBUG MESSAGE] ");

  // print error severity
  switch (severity) {
  case GL_DEBUG_SEVERITY_LOW:
    msg.append("\u001b[32m<Low severity> \u001b[0m");

    break;
  case GL_DEBUG_SEVERITY_MEDIUM:
    msg.append("\u001b[33m<Medium severity> \u001b[0m");
    break;
  case GL_DEBUG_SEVERITY_HIGH:
    msg.append("\u001b[31m<High severity> \u001b[0m");
    break;
  }

  // append message to output
  msg.append(message);

  // print message
  switch (type) {
  case GL_DEBUG_TYPE_ERROR:
  case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
    printf("Error: %s\n", msg.c_str());
    break;
  case GL_DEBUG_TYPE_PERFORMANCE:
    printf("Performance issue: %s\n", msg.c_str());
    break;
  default: // Portability, Deprecated, Other
    break;
  }
}
#endif

void glfw_error(int /* error */, const char *description) {
  std::cout << description << std::endl;
}

bool Window::start() {
  std::cout << "[window] Start" << std::endl;
  //glfwSetErrorCallback(glfw_error);

  if (glfwInit()) {
#ifdef _DEBUG_
    std::cout << "DEBUG MODE" << std::endl;
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
    glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_API);
#else
    glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
#endif
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_ANY_PROFILE);
    // glfwWindowHint(GLFW_SAMPLES, 4);

    return true;
  }

  std::cout << "[window] Failed to init glfw!" << std::endl;
  return false;
}

bool Window::open() {

  std::cout << "[window] Open" << std::endl;
  m_window = glfwCreateWindow(m_width, m_height, "Dagon", NULL, NULL);
  if (m_window == NULL) {
    std::cout << "Failed to create GLFW window" << std::endl;
    glfwTerminate();
    return false;
  }
  glfwMakeContextCurrent(m_window);
  glfwSwapInterval(0);
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGuiIO &io = ImGui::GetIO();
  (void)io;

  io.IniFilename = NULL;

  // Setup Dear ImGui style
  ImGui::StyleColorsDark();

  // Setup Platform/Renderer backends
  ImGui_ImplGlfw_InitForOpenGL(m_window, true);
  ImGui_ImplOpenGL3_Init("#version 300 es");

#ifndef EMSCRIPTEN
  if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
    std::cout << "Failed to initialize GLAD" << std::endl;
    return false;
  }
#endif

#ifdef _DEBUG_
  glEnable(GL_DEBUG_OUTPUT);
  glDebugMessageCallback(MessageCallback, 0);
#endif

  return true;
}

bool Window::closed() { return glfwWindowShouldClose(m_window); }

bool Window::close() {
  std::cout << "[window] Close" << std::endl;
  glfwSetWindowShouldClose(m_window, true);
  return true;
}

void Window::swap() {
  // ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
  glfwSwapBuffers(m_window);
}

void Window::setCursorPosCallback(void (*callback)(GLFWwindow *window,
                                                   double xpos, double ypos)) {
  glfwSetCursorPosCallback(m_window, callback);
}
void Window::setMouseButtonCallback(void (*callback)(GLFWwindow *win,
                                                     i32 button, i32 action,
                                                     i32 mods)) {
  glfwSetMouseButtonCallback(m_window, callback);
}
void Window::setKeyCallback(void (*callback)(GLFWwindow *win, i32 key,
                                             i32 scancode, i32 action,
                                             i32 mods)) {
  glfwSetKeyCallback(m_window, callback);
}

void Window::setFramebufferSizeCallback(void (*callback)(GLFWwindow *window,
                                                         i32 width,
                                                         i32 height)) {
  glfwSetFramebufferSizeCallback(m_window, callback);
}
