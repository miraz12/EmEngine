#ifndef ENGINE_API_HPP_
#define ENGINE_API_HPP_
enum KEY {
  Escape,
  W,
  A,
  S,
  D,
  F,
  O,
  Space,
  ArrowUp,
  ArrowDown,
  ArrowRight,
  ArrowLeft,
  Mouse1
};

extern void ECS_loadScene(const char *file);
extern int Input_getPressed(void *vec);
extern int Input_getReleased(void *vec);

#endif // ENGINE_API_HPP_
