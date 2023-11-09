#ifndef GUI_H_
#define GUI_H_

#include <Game.hpp>

class GUI {

public:
  GUI() = delete;
  GUI(Game &game);
  ~GUI() = default;
  void renderGUI();
  void renderGizmos();

private:
  void editTransform(float *cameraView, float *cameraProjection, float *matrix,
                     bool editTransformDecomposition);

  Game &m_game;
  bool useWindow = true;
  int gizmoCount = 1;
  float camDistance = 8.f;
  float objectMatrix[4][16] = {{1.f, 0.f, 0.f, 0.f, 0.f, 1.f, 0.f, 0.f, 0.f,
                                0.f, 1.f, 0.f, 0.f, 0.f, 0.f, 1.f},

                               {1.f, 0.f, 0.f, 0.f, 0.f, 1.f, 0.f, 0.f, 0.f,
                                0.f, 1.f, 0.f, 2.f, 0.f, 0.f, 1.f},

                               {1.f, 0.f, 0.f, 0.f, 0.f, 1.f, 0.f, 0.f, 0.f,
                                0.f, 1.f, 0.f, 2.f, 0.f, 2.f, 1.f},

                               {1.f, 0.f, 0.f, 0.f, 0.f, 1.f, 0.f, 0.f, 0.f,
                                0.f, 1.f, 0.f, 0.f, 0.f, 2.f, 1.f}};
};

#endif // GUI_H_
