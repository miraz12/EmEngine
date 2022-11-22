#include "Window.hpp"

#ifdef EMSCRIPTEN
#define GL_OES_vertex_array_object
#include <GLES3/gl3.h>
#include <emscripten.h>
#else
#include <glad/glad.h>
#endif

#include <GLFW/glfw3.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>

#include <Game/Game.hpp>
#include <fstream>
#include <iostream>

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

void framebuffer_size_callback(GLFWwindow *window, int width, int height);
void processInput(GLFWwindow *window);
void errorCallback(int /*error*/, const char *description) {
  printf("Error: %s\n", description);
}

static GLFWwindow *window;
static Game *game;

bool isBigEndian() {
  int a = 1;
  return !((char *)&a)[0];
}

int convertToInt(char *buffer, int len) {
  int a = 0;
  if (!isBigEndian())
    for (int i = 0; i < len; i++) ((char *)&a)[i] = buffer[i];
  else
    for (int i = 0; i < len; i++) ((char *)&a)[3 - i] = buffer[i];
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
  in.read(buffer, 4);  // WAVE
  in.read(buffer, 4);  // fmt
  in.read(buffer, 4);  // 16
  in.read(buffer, 2);  // 1
  in.read(buffer, 2);
  chan = convertToInt(buffer, 2);
  in.read(buffer, 4);
  samplerate = convertToInt(buffer, 4);
  in.read(buffer, 4);
  in.read(buffer, 2);
  in.read(buffer, 2);
  bps = convertToInt(buffer, 2);
  in.read(buffer, 4);  // data
  in.read(buffer, 4);
  size = convertToInt(buffer, 4);
  char *data = new char[size];
  in.read(data, size);
  return data;
}

bool Window::open() {
  glfwInit();
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  glfwWindowHint(GLFW_SAMPLES, 4);

  glfwSetErrorCallback(errorCallback);
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
  glfwSwapInterval(0);

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
    // std::cout << fps;
    glfwSetWindowTitle(window,
                       ("OpenGL FPS: " + std::to_string((int)fps)).c_str());
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

  if (updatesSinceRender == 0) {  // dt is faster than
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
  // Define a mail loop function, that will be called as fast as possible
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
  }
}

void framebuffer_size_callback(GLFWwindow * /*window*/, int width, int height) {
  SCR_WIDTH = width;
  SCR_HEIGHT = height;
  glViewport(0, 0, width, height);
}

void Window::renderImgui() {
  // Start the Dear ImGui frame

  ImGui_ImplOpenGL3_NewFrame();
  ImGui_ImplGlfw_NewFrame();
  ImGui::NewFrame();

  static float f = 0.0f;

  ImGui::Begin("Hello, world!");  // Create a window called "Hello, world!"
  ImGui::Text("This is some useful text.");
  ImGui::SliderFloat("float", &f, 0.0f,
                     1.0f);  // Edit 1 float using a slider from 0.0f to 1.0f

  ImGui::End();
  // Rendering
  ImGui::Render();
}
