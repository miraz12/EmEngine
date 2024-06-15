#ifndef GAME_API_H_
#define GAME_API_H_

extern "C" void game_initialize();
extern "C" void game_update(float dt);
extern "C" int InitializeFunctions(void (*func)(const char *),
                                   int (*func2)(void *));

#endif // GAME_API_H_
