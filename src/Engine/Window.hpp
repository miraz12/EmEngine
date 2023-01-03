#pragma once

class Window {
public:
  Window() = default;
  ~Window() = default;

  static bool start();
  static bool open();
  static void gameLoop();
  static void renderImgui();
};
