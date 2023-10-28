#include "Window.hpp"

#ifdef EMSCRIPTEN
#define GL_OES_vertex_array_object
#include <GLES3/gl3.h>
#include <emscripten.h>
#else
#include <glad/glad.h>
// #define _DEBUG_
#endif

#include <GLFW/glfw3.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>

#include <Game.hpp>
#include <fstream>
#include <iostream>

#include "InputManager.hpp"

static double currentTime;
static double previousTime;
static double dt = 0.0;

static float fps = 0.0f;
static float tempFps = 0.0f;
static i32 counter = 0;
static float fpsUpdate = 1.0f;
static float fpsUpdateTimer = 0.0f;
static float fpsArray[50];
static size_t fpsIdx;

static u32 SCR_WIDTH = 800;
static u32 SCR_HEIGHT = 800;
static double SCR_PITCH = 0.0;
static double SCR_YAW = -90.0;

double lastX, lastY;

static InputManager &inMgr = InputManager::getInstance();

void mouse_callback(GLFWwindow *window, double xpos, double ypos);
void framebuffer_size_callback(GLFWwindow *window, i32 width, i32 height);
void keyPressCallback(GLFWwindow *win, i32 key, i32 scancode, i32 action,
                      i32 mods);
void mousePressCallback(GLFWwindow *win, i32 button, i32 action, i32 mods);

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

static GLFWwindow *window;
static Game *game;

bool isBigEndian() {
  i32 a = 1;
  return !((char *)&a)[0];
}

i32 convertToInt(char *buffer, i32 len) {
  i32 a = 0;
  if (!isBigEndian())
    for (u32 i = 0; i < len; i++)
      ((char *)&a)[i] = buffer[i];
  else
    for (u32 i = 0; i < len; i++)
      ((char *)&a)[3 - i] = buffer[i];
  return a;
}

char *loadWAV(const char *fn, i32 &chan, i32 &samplerate, i32 &bps, i32 &size) {
  char buffer[4];
  std::ifstream in(fn, std::ios::binary);
  in.read(buffer, 4);
  if (strncmp(buffer, "RIFF", 4) != 0) {
    std::cout << "this is not a valid WAVE file" << std::endl;
    return NULL;
  }
  in.read(buffer, 4);
  in.read(buffer, 4); // WAVE
  in.read(buffer, 4); // fmt
  in.read(buffer, 4); // 16
  in.read(buffer, 2); // 1
  in.read(buffer, 2);
  chan = convertToInt(buffer, 2);
  in.read(buffer, 4);
  samplerate = convertToInt(buffer, 4);
  in.read(buffer, 4);
  in.read(buffer, 2);
  in.read(buffer, 2);
  bps = convertToInt(buffer, 2);
  in.read(buffer, 4); // data
  in.read(buffer, 4);
  size = convertToInt(buffer, 4);
  char *data = new char[size];
  in.read(data, size);
  return data;
}

bool Window::open() {
  glfwInit();
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

  SCR_WIDTH = 800;
  SCR_HEIGHT = 800;
  window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", NULL, NULL);
  if (window == NULL) {
    std::cout << "Failed to create GLFW window" << std::endl;
    glfwTerminate();
    return false;
  }
  glfwMakeContextCurrent(window);
  glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
  glfwSetCursorPosCallback(window, mouse_callback);
  glfwSetKeyCallback(window, keyPressCallback);
  glfwSetMouseButtonCallback(window, mousePressCallback);

  // Setup IMGUI
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGuiIO &io = ImGui::GetIO();
  (void)io;

  io.IniFilename = NULL;

  // Setup Dear ImGui style
  ImGui::StyleColorsDark();

  // Setup Platform/Renderer backends
  ImGui_ImplGlfw_InitForOpenGL(window, true);
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

void Window::gameLoop() {
  // input
  // -----
  glfwPollEvents();

  renderImgui();

  // Update
  // -----
  currentTime = glfwGetTime();
  dt = currentTime - previousTime;
  previousTime = currentTime;

  if (fpsUpdateTimer <= fpsUpdate) {
    fpsUpdateTimer += (float)dt;
    tempFps += (1 / (float)dt);
    counter++;
  } else {
    fps = tempFps / counter;
    tempFps = 0.0f;
    counter = 0;
    fpsUpdateTimer = 0.0f;
    glfwSetWindowTitle(window,
                       ("OpenGL FPS: " + std::to_string((u32)fps)).c_str());
  }

  if (fpsIdx == 50) {
    fpsIdx = 0;
  }
  fpsArray[fpsIdx++] = fps;

  game->update((float)dt);

  ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
  glfwSwapBuffers(window);
}

bool Window::start() {
  currentTime = glfwGetTime();
  previousTime = currentTime;

  game = new Game(*window);

#ifdef EMSCRIPTEN
  emscripten_set_main_loop(&gameLoop, 0, 1);
#else

  // render loop
  // -----------
  while (!glfwWindowShouldClose(window)) {
    gameLoop();
  }
#endif

  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplGlfw_Shutdown();
  ImGui::DestroyContext();

  glfwDestroyWindow(window);
  glfwTerminate();
  return 0;
}

void mouse_callback(GLFWwindow * /* window */, double xpos, double ypos) {

  if (!inMgr.keys.at(InputManager::KEY::Mouse1)) {
    lastX = xpos;
    lastY = ypos;
    return;
  }

  float xoffset = xpos - lastX;
  float yoffset = lastY - ypos;
  lastX = xpos;
  lastY = ypos;

  float sensitivity = 0.1f;
  xoffset *= sensitivity;
  yoffset *= sensitivity;

  SCR_YAW += xoffset;
  SCR_PITCH += yoffset;

  if (SCR_PITCH > 89.0f)
    SCR_PITCH = 89.0f;
  if (SCR_PITCH < -89.0f)
    SCR_PITCH = -89.0f;

  game->setPitchYaw(SCR_PITCH, SCR_YAW);
}

void keyPressCallback(GLFWwindow *win, i32 key, i32 /* scancode */, i32 action,
                      i32 /* mods */) {
  switch (key) {
  case GLFW_KEY_ESCAPE:
    glfwSetWindowShouldClose(win, true);
    break;
  case GLFW_KEY_W:
    inMgr.handleInput(InputManager::KEY::W, action);
    break;
  case GLFW_KEY_A:
    inMgr.handleInput(InputManager::KEY::A, action);
    break;
  case GLFW_KEY_S:
    inMgr.handleInput(InputManager::KEY::S, action);
    break;
  case GLFW_KEY_D:
    inMgr.handleInput(InputManager::KEY::D, action);
    break;
  case GLFW_KEY_F:
    inMgr.handleInput(InputManager::KEY::F, action);
    break;
  case GLFW_KEY_O:
    inMgr.handleInput(InputManager::KEY::O, action);
    break;
  case GLFW_KEY_SPACE:
    inMgr.handleInput(InputManager::KEY::Space, action);
    break;
  case GLFW_KEY_UP:
    inMgr.handleInput(InputManager::KEY::ArrowUp, action);
    break;
  case GLFW_KEY_DOWN:
    inMgr.handleInput(InputManager::KEY::ArrowDown, action);
    break;
  case GLFW_KEY_RIGHT:
    inMgr.handleInput(InputManager::KEY::ArrowRight, action);
    break;
  case GLFW_KEY_LEFT:
    inMgr.handleInput(InputManager::KEY::ArrowLeft, action);
    break;
  default:
    break;
  }
}

void mousePressCallback(GLFWwindow * /* win */, i32 button, i32 action,
                        i32 /* mods */) {
  ImGuiIO &io = ImGui::GetIO();
  io.AddMouseButtonEvent(button, action);
  if (!io.WantCaptureMouse) {
    inMgr.handleInput(InputManager::KEY::Mouse1, action);
  }

  double xpos, ypos;
  glfwGetCursorPos(window, &xpos, &ypos);
  game->setMousePos(xpos, ypos);
}

void framebuffer_size_callback(GLFWwindow * /*window*/, i32 width, i32 height) {
  SCR_WIDTH = width;
  SCR_HEIGHT = height;
  game->setViewport(width, height);
  glViewport(0, 0, width, height);
}

void Window::renderImgui() {
  // Start the Dear ImGui frame

  ImGui_ImplOpenGL3_NewFrame();
  ImGui_ImplGlfw_NewFrame();
  ImGui::NewFrame();
  ImGui::Begin("Settings", 0, ImGuiWindowFlags_AlwaysAutoResize);

  if (ImGui::CollapsingHeader("Lights")) {
    ImGui::SliderFloat3("Direction", glm::value_ptr(game->dirLightDir), -1.0f,
                        1.0f);
    ImGui::SliderFloat("Ambient", &game->dirLightAmbient, 0.0f, 2.0f);
    ImGui::ColorEdit3("Color", glm::value_ptr(game->dirLightColor));
  }

  if (ImGui::CollapsingHeader("Physics")) {
    ImGui::Checkbox("Enabled", &ECSManager::getInstance().simulatePhysics);
    Entity en = game->m_ECSManager.getPickedEntity();
    if (game->m_ECSManager.getEntitySelected()) {
      ImGui::Text("Selected entity: %lu", en);
      glm::vec3 pos =
          game->m_ECSManager.getComponent<PositionComponent>(en)->position;
      ImGui::Text("Position: X: %f Y: %f Z: %f ", pos.x, pos.y, pos.z);
    }
  }

  static i32 offset = 0;
  offset = (offset + 1) % 50;
  if (ImGui::CollapsingHeader("Debug")) {
    ImGui::PlotLines("FPS", fpsArray, 50, offset, nullptr, 0, 60,
                     ImVec2(0, 80.f));

    const std::vector<std::string> debugNamesInputs = {
        "none",     "Base color", "Normal",   "Occlusion",
        "Emissive", "Metallic",   "Roughness"};
    std::vector<const char *> charitems;
    charitems.reserve(debugNamesInputs.size());
    for (size_t i = 0; i < debugNamesInputs.size(); i++) {
      charitems.push_back(debugNamesInputs[i].c_str());
    }
    ImGui::Combo("views", &ECSManager::getInstance().debugView, &charitems[0],
                 7, 7);
  }

  ImGui::End();
  // Rendering
  ImGui::Render();
}
