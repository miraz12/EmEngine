#pragma once

static unsigned int SCR_WIDTH = 800;
static unsigned int SCR_HEIGHT = 800;

class Window {
public:
  Window() = default;
  ~Window() = default;

 static bool start();
 static bool setup();
 static bool open();
 static void gameLoop();
 static void renderImgui();
};
