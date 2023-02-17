#include "Window.hpp"

#ifdef EMSCRIPTEN
#define GL_OES_vertex_array_object
#include <GLES3/gl3.h>
#include <emscripten.h>
#else
#include <glad/glad.h>
#define _DEBUG_
#endif

#include <GLFW/glfw3.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>

#include <Game/Game.hpp>
#include <fstream>
#include <iostream>

#include "InputManager.hpp"

static double currentTime;
static double previousTime;
static double dt = 0.0;

static float fps = 0.0f;
static float tempFps = 0.0f;
static int counter = 0;
static float fpsUpdate = 1.0f;
static float fpsUpdateTimer = 0.0f;

static double minUpdateRate = 1.0f / 60.0f;
static double updateTimer = 0.0f;
static int updatesSinceRender = 0;

static unsigned int SCR_WIDTH = 800;
static unsigned int SCR_HEIGHT = 800;
static double SCR_PITCH = 0.0;
static double SCR_YAW = -90.0;

double lastX, lastY;

static InputManager &inMgr = InputManager::getInstance();


void mouse_callback(GLFWwindow *window, double xpos, double ypos);
void framebuffer_size_callback(GLFWwindow *window, int width, int height);
void processInput(GLFWwindow *window);

#ifndef EMSCRIPTEN
void GLAPIENTRY MessageCallback(GLenum /* source */, GLenum type, GLuint /* id */, GLenum severity,
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
  int a = 1;
  return !((char *)&a)[0];
}

int convertToInt(char *buffer, int len) {
  int a = 0;
  if (!isBigEndian())
    for (int i = 0; i < len; i++)
      ((char *)&a)[i] = buffer[i];
  else
    for (int i = 0; i < len; i++)
      ((char *)&a)[3 - i] = buffer[i];
  return a;
}

char *loadWAV(const char *fn, int &chan, int &samplerate, int &bps, int &size) {
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
  glfwWindowHint(GLFW_SAMPLES, 4);

  SCR_WIDTH = 1000;
  SCR_HEIGHT = 1000;
  window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", NULL, NULL);
  if (window == NULL) {
    std::cout << "Failed to create GLFW window" << std::endl;
    glfwTerminate();
    return false;
  }
  glfwMakeContextCurrent(window);
  glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
  glfwSetCursorPosCallback(window, mouse_callback);

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
  processInput(window);

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
    glfwSetWindowTitle(window, ("OpenGL FPS: " + std::to_string((int)fps)).c_str());
  }

  updateTimer += dt;
  updatesSinceRender = 0;

  // If dt is bigger than minUpdateRate - update multiple times
  while (updateTimer >= minUpdateRate) {
    if (updatesSinceRender >= 20) {
      // Too many updates, throw away the rest of dt (makes the game run in
      // slow-motion)
      updateTimer = 0.0f;
      break;
    }

    game->update((float)minUpdateRate);
    updateTimer -= minUpdateRate;
    updatesSinceRender++;
  }

  if (updatesSinceRender == 0) { // dt is faster than
    game->update((float)updateTimer);
    updateTimer = 0.0f;
  }

  ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
  glfwSwapBuffers(window);
}

bool Window::start() {
  currentTime = glfwGetTime();
  previousTime = currentTime;

  game = new Game(window);

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
  return true;
}

void processInput(GLFWwindow *theWindow) {
  if (glfwGetKey(theWindow, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
    glfwSetWindowShouldClose(theWindow, true);
    exit(0);
  }

  inMgr.HandleInput(InputManager::KEY::Escape, glfwGetKey(theWindow, GLFW_KEY_ESCAPE));
  inMgr.HandleInput(InputManager::KEY::W, glfwGetKey(theWindow, GLFW_KEY_W));
  inMgr.HandleInput(InputManager::KEY::A, glfwGetKey(theWindow, GLFW_KEY_A));
  inMgr.HandleInput(InputManager::KEY::S, glfwGetKey(theWindow, GLFW_KEY_S));
  inMgr.HandleInput(InputManager::KEY::D, glfwGetKey(theWindow, GLFW_KEY_D));
  inMgr.HandleInput(InputManager::KEY::F, glfwGetKey(theWindow, GLFW_KEY_F));
  inMgr.HandleInput(InputManager::KEY::ArrowUp, glfwGetKey(theWindow, GLFW_KEY_UP));
  inMgr.HandleInput(InputManager::KEY::ArrowDown, glfwGetKey(theWindow, GLFW_KEY_DOWN));
  inMgr.HandleInput(InputManager::KEY::ArrowRight, glfwGetKey(theWindow, GLFW_KEY_RIGHT));
  inMgr.HandleInput(InputManager::KEY::ArrowLeft, glfwGetKey(theWindow, GLFW_KEY_LEFT));
  inMgr.HandleInput(InputManager::KEY::Mouse1, glfwGetMouseButton(theWindow, GLFW_MOUSE_BUTTON_1));
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

void framebuffer_size_callback(GLFWwindow * /*window*/, int width, int height) {
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

  static float f = 0.0f;

  ImGui::Begin("Hello, world!"); // Create a window called "Hello, world!"
  ImGui::Text("This is some useful text.");
  ImGui::SliderFloat("float", &f, 0.0f,
                     1.0f); // Edit 1 float using a slider from 0.0f to 1.0f

  ImGui::End();
  // Rendering
  ImGui::Render();
}
