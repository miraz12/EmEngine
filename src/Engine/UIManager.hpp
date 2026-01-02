#ifndef UIMANAGER_HPP_
#define UIMANAGER_HPP_

#include "Singleton.hpp"
#include <RmlUi/Core.h>

struct GLFWwindow;

namespace Rml {
class Context;
class RenderInterface;
class SystemInterface;
}

/**
 * @brief Manages RmlUi-based user interface rendering and interaction
 *
 * UIManager is a singleton that handles:
 * - RmlUi context creation and lifecycle
 * - Document loading and management
 * - Integration with GLFW for input and rendering
 * - OpenGL 3.3 renderer backend
 */
class UIManager : public Singleton<UIManager>
{
  friend class Singleton<UIManager>;

public:
  /**
   * @brief Initialize RmlUi with the GLFW window
   * @param window GLFW window handle
   * @param width Window width
   * @param height Window height
   * @return true if initialization succeeded
   */
  bool initialize(GLFWwindow* window, int width, int height);

  /**
   * @brief Clean up RmlUi resources
   */
  void shutdown();

  /**
   * @brief Update UI (called per frame before rendering)
   * @param dt Delta time in seconds
   */
  void update(float dt);

  /**
   * @brief Render all UI documents
   */
  void render();

  /**
   * @brief Load and display a document from file
   * @param path Path to RML file (relative to resources/ui/)
   * @param name Unique name for the document
   * @return true if document loaded successfully
   */
  bool loadDocument(const std::string& path, const std::string& name);

  /**
   * @brief Show a previously loaded document
   * @param name Document name
   */
  void showDocument(const std::string& name);

  /**
   * @brief Hide a document
   * @param name Document name
   */
  void hideDocument(const std::string& name);

  /**
   * @brief Unload a document
   * @param name Document name
   */
  void unloadDocument(const std::string& name);

  /**
   * @brief Check if a document is currently visible
   * @param name Document name
   * @return true if document exists and is visible
   */
  bool isDocumentVisible(const std::string& name) const;

  /**
   * @brief Handle window resize events
   * @param width New window width
   * @param height New window height
   */
  void onResize(int width, int height);

  /**
   * @brief Process mouse movement
   * @param x Mouse X position
   * @param y Mouse Y position
   * @param modifiers Key modifiers
   */
  void onMouseMove(int x, int y, int modifiers);

  /**
   * @brief Process mouse button events
   * @param button Mouse button index
   * @param action GLFW action (press/release)
   * @param modifiers Key modifiers
   */
  void onMouseButton(int button, int action, int modifiers);

  /**
   * @brief Process mouse scroll events
   * @param yoffset Scroll offset
   * @param modifiers Key modifiers
   */
  void onMouseScroll(double yoffset, int modifiers);

  /**
   * @brief Process keyboard events
   * @param key GLFW key code
   * @param action GLFW action (press/release/repeat)
   * @param modifiers Key modifiers
   */
  void onKey(int key, int action, int modifiers);

  /**
   * @brief Process text input
   * @param codepoint Unicode codepoint
   */
  void onTextInput(unsigned int codepoint);

  /**
   * @brief Check if UI wants to capture mouse input
   * @return true if UI is handling mouse events
   */
  bool wantsMouseCapture() const;

  /**
   * @brief Check if UI wants to capture keyboard input
   * @return true if UI is handling keyboard events
   */
  bool wantsKeyboardCapture() const;

  /**
   * @brief Get the RmlUi context
   * @return Pointer to the context (may be null)
   */
  Rml::Context* getContext() { return m_context; }

  /**
   * @brief Register a click callback for a UI element
   * @param documentName Name of the document containing the element
   * @param elementId ID of the element to attach callback to
   * @param callback Function to call when element is clicked
   * @return true if callback was registered successfully
   */
  using ClickCallback = std::function<void()>;
  bool registerClickCallback(const std::string& documentName,
                             const std::string& elementId,
                             ClickCallback callback);

private:
  UIManager() = default;
  ~UIManager();

  // Custom event listener for UI events
  class UIEventListener : public Rml::EventListener
  {
  public:
    UIEventListener(ClickCallback callback)
      : m_callback(callback)
    {
    }

    void ProcessEvent(Rml::Event& event) override
    {
      if (m_callback) {
        m_callback();
      }
    }

  private:
    ClickCallback m_callback;
  };

  Rml::Context* m_context = nullptr;
  std::unique_ptr<Rml::RenderInterface> m_renderInterface;
  std::unique_ptr<Rml::SystemInterface> m_systemInterface;

  struct DocumentInfo
  {
    Rml::ElementDocument* document;
    std::string path;
  };
  std::unordered_map<std::string, DocumentInfo> m_documents;

  // Storage for event listeners to keep them alive
  std::vector<std::unique_ptr<UIEventListener>> m_eventListeners;

  bool m_initialized = false;
  int m_width = 0;
  int m_height = 0;
};

#endif // UIMANAGER_HPP_
