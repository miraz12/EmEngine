#ifndef WINDOW_H_
#define WINDOW_H_

#include <Singleton.hpp>

class Window : public Singleton<Window> {
  friend class Singleton<Window>;

public:
  Window() = default;
  ~Window() = default;

  bool start();
  bool open();
  bool close();
  bool closed();
  void swap();

  void setCursorPosCallback(void (*callback)(GLFWwindow *win, double xpos,
                                             double ypos));
  void setMouseButtonCallback(void (*callback)(GLFWwindow *win, i32 button,
                                               i32 action, i32 mods));
  void setKeyCallback(void (*callback)(GLFWwindow *win, i32 key, i32 scancode,
                                       i32 action, i32 mods));
  void setFramebufferSizeCallback(void (*callback)(GLFWwindow *window,
                                                   i32 width, i32 height));

  GLFWwindow *getWindow() { return m_window; };

private:
  u32 m_width = 800;
  u32 m_height = 800;
  GLFWwindow *m_window;
};

#endif // WINDOW_H_
