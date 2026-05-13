#ifndef ENGINE_API_HPP_
#define ENGINE_API_HPP_
#include "Core.hpp"
#include "ECS/ECSManager.hpp"
#include "GameStateManager.hpp"
#include "MapLoader.hpp"
#include "UIManager.hpp"

extern "C"
{

  void LoadScene(const char* filename)
  {
    SceneLoader::getInstance().init(filename);
  }

  void LoadMap(const char* filename, float tileSize)
  {
    MapLoader::getInstance().loadMap(filename, tileSize);
  }

  void PauseAnimation(unsigned int entity);
  void StartAnimation(unsigned int entity);
  void SetAnimationIndex(unsigned int entity, unsigned int idx);
  void CrossfadeAnimation(unsigned int entity,
                          unsigned int targetIndex,
                          float duration);
  void SetRotation(unsigned int entity, float angle);
  void AddPositionComponent(int entity,
                            float pos[3],
                            float scale[3],
                            float rot[3]);
  void AddPhysicsComponent(int entity, float mass, int type);
  void AddGraphicsComponent(int entity, const char* model);
  void AddCameraComponent(int entity, bool main, float offset[3]);
  void SetMainCamera(int entity);
  void SetCameraOrientation(int entity,
                            float frontX,
                            float frontY,
                            float frontZ,
                            float upX,
                            float upY,
                            float upZ);
  int CreateEntity(const char* name);
  int GetPressed(int* vec);
  void GetMousePosition(float* x, float* y);
  void GetMouseDelta(float* deltaX, float* deltaY);
  bool GetSimulatePhysics();
  void Game_Update();
  bool Initialize()
  {
    return Core::getInstance().initialize();
  }
  bool Open()
  {
    return Core::getInstance().open();
  };
  void Update()
  {
    Core::getInstance().update();
  };
  void SetVelocity(unsigned int entity, float x, float y, float z);
  void SetHorizontalVelocity(unsigned int entity, float x, float z);
  void GetVelocity(unsigned int entity, float* velocity);
  void AddImpulse(unsigned int entity, float x, float y, float z);
  void AddForce(unsigned int entity, float x, float y, float z);
  bool EntityOnGround(unsigned int entity);

  // Entity Management API
  void DestroyEntity(unsigned int entity);
  int GetEntityCount();
  unsigned int GetEntityAtIndex(int index);
  bool HasComponent(unsigned int entity, int componentType);

  // Transform API
  void GetPosition(unsigned int entity, float* outX, float* outY, float* outZ);
  void SetPosition(unsigned int entity, float x, float y, float z);
  void GetScale(unsigned int entity, float* outX, float* outY, float* outZ);
  void SetScale(unsigned int entity, float x, float y, float z);
  void GetRotationQuat(unsigned int entity,
                       float* outX,
                       float* outY,
                       float* outZ,
                       float* outW);
  void SetRotationQuat(unsigned int entity, float x, float y, float z, float w);

  // Camera API
  void SetCameraFov(unsigned int entity, float fov);
  void SetCameraFarPlane(unsigned int entity, float farPlane);
  void SetCameraNearPlane(unsigned int entity, float nearPlane);
  void GetCameraPosition(unsigned int entity,
                         float* outX,
                         float* outY,
                         float* outZ);

  // Physics API
  void SetGravity(float x, float y, float z);
  void SetSimulatePhysics(bool simulate);
  void SetPhysicsTransform(unsigned int entity,
                           float posX,
                           float posY,
                           float posZ,
                           float rotX,
                           float rotY,
                           float rotZ,
                           float rotW);

  // Lighting API
  void AddPointLight(unsigned int entity,
                     float r,
                     float g,
                     float b,
                     float constant,
                     float linear,
                     float quadratic,
                     float x,
                     float y,
                     float z);
  void AddDirectionalLight(unsigned int entity,
                           float r,
                           float g,
                           float b,
                           float intensity,
                           float dirX,
                           float dirY,
                           float dirZ);
  void UpdateDirectionalLight(float r,
                              float g,
                              float b,
                              float intensity,
                              float dirX,
                              float dirY,
                              float dirZ);
  void SetPointLightColor(unsigned int entity, float r, float g, float b);
  void SetPointLightPosition(unsigned int entity, float x, float y, float z);

  // Particles API
  void AddParticlesComponent(unsigned int entity,
                             float velX,
                             float velY,
                             float velZ);
  void SetParticleVelocity(unsigned int entity, float x, float y, float z);
  void SetParticleRate(unsigned int entity, unsigned int rate);

  // ECS Reset
  void ResetECS();

  // Core API
  float GetDeltaTime()
  {
    return Core::getInstance().getDt();
  }

  // Input / Window API
  void SetCursorMode(int mode);
  void GetWindowSize(int* outWidth, int* outHeight);

  // UI API functions
  bool UI_LoadDocument(const char* path, const char* name)
  {
    return UIManager::getInstance().loadDocument(path, name);
  }

  void UI_ShowDocument(const char* name)
  {
    UIManager::getInstance().showDocument(name);
  }

  void UI_HideDocument(const char* name)
  {
    UIManager::getInstance().hideDocument(name);
  }

  void UI_UnloadDocument(const char* name)
  {
    UIManager::getInstance().unloadDocument(name);
  }

  bool UI_IsDocumentVisible(const char* name)
  {
    return UIManager::getInstance().isDocumentVisible(name);
  }

  // Callback type for UI and state events
  using StateChangeCallback = void (*)();

  bool UI_RegisterClickCallback(const char* documentName,
                                const char* elementId,
                                StateChangeCallback callback)
  {
    return UIManager::getInstance().registerClickCallback(
      documentName, elementId, [callback]() { callback(); });
  }

  // Game State API functions
  void GameState_StartGame()
  {
    GameStateManager::getInstance().startGame();
  }

  void GameState_PauseGame()
  {
    GameStateManager::getInstance().pauseGame();
  }

  void GameState_ResumeGame()
  {
    GameStateManager::getInstance().resumeGame();
  }

  void GameState_ReturnToMainMenu()
  {
    GameStateManager::getInstance().returnToMainMenu();
  }

  void GameState_GameOver()
  {
    GameStateManager::getInstance().gameOver();
  }

  int GameState_GetCurrentState()
  {
    return static_cast<int>(GameStateManager::getInstance().getCurrentState());
  }

  bool GameState_IsPlaying()
  {
    return GameStateManager::getInstance().isPlaying();
  }

  bool GameState_IsPaused()
  {
    return GameStateManager::getInstance().isPaused();
  }

  void GameState_OnEnterPlaying(StateChangeCallback callback)
  {
    GameStateManager::getInstance().onEnterState(GameState::Playing,
                                                 [callback]() { callback(); });
  }

  void GameState_OnExitPlaying(StateChangeCallback callback)
  {
    GameStateManager::getInstance().onExitState(GameState::Playing,
                                                [callback]() { callback(); });
  }

  // Audio API
  void AddAudioSourceComponent(int entity, const char* clipPath);
  void Audio_Play(unsigned int entity);
  void Audio_Stop(unsigned int entity);
  void Audio_SetVolume(unsigned int entity, float volume);
  void Audio_SetPitch(unsigned int entity, float pitch);
  void Audio_SetLoop(unsigned int entity, bool loop);
  void Audio_Set3D(unsigned int entity, bool is3D);
  void Audio_SetMinDistance(unsigned int entity, float dist);
  void Audio_SetMaxDistance(unsigned int entity, float dist);
  bool Audio_IsPlaying(unsigned int entity);
  void Audio_SetMasterVolume(float volume);
  float Audio_GetMasterVolume();
  void Audio_LoadClip(const char* path);

  void Start()
  {

#ifdef EMSCRIPTEN
    emscripten_set_main_loop(&Game_Update, 0, 1);
#else
    while (Core::getInstance().open()) {
      Game_Update();
    }
#endif
  }
};

#endif // ENGINE_API_HPP_
