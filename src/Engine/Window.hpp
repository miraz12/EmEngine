#ifndef WINDOW_H_
#define WINDOW_H_

class Window {
 public:
  Window() = default;
  ~Window() = default;

  static bool start();
  static bool open();
  static void gameLoop();
  static void renderImgui();
};

#endif  // WINDOW_H_
