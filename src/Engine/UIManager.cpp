#include "UIManager.hpp"
#include <GLFW/glfw3.h>
#include <RmlUi/Core.h>
#include <RmlUi_Platform_GLFW.h>
#include <RmlUi_Renderer_GL3.h>
#include <iostream>

UIManager::~UIManager()
{
  if (m_initialized) {
    shutdown();
  }
}

bool
UIManager::initialize(GLFWwindow* window, int width, int height)
{
  if (m_initialized) {
    std::cerr << "UIManager already initialized\n";
    return false;
  }

  m_width = width;
  m_height = height;

  // Create system interface
  auto systemInterface = std::make_unique<SystemInterface_GLFW>();
  systemInterface->SetWindow(window);
  m_systemInterface = std::move(systemInterface);
  Rml::SetSystemInterface(m_systemInterface.get());

  // Create render interface
  auto renderInterface = std::make_unique<RenderInterface_GL3>();

  // Check if renderer initialized successfully
  if (!*renderInterface) {
    std::cerr << "Failed to initialize RmlUi GL3 renderer \n";
    return false;
  }

  renderInterface->SetViewport(width, height);
  m_renderInterface = std::move(renderInterface);
  Rml::SetRenderInterface(m_renderInterface.get());

  // Initialize RmlUi
  if (!Rml::Initialise()) {
    std::cerr << "Failed to initialize RmlUi \n";
    return false;
  }

  // Create context
  m_context = Rml::CreateContext("main", Rml::Vector2i(width, height));
  if (!m_context) {
    std::cerr << "Failed to create RmlUi context\n";
    Rml::Shutdown();
    return false;
  }

  // Load default font
  if (!Rml::LoadFontFace("resources/fonts/LatoLatin-Regular.ttf")) {
    std::cerr << "Warning: Failed to load default font\n";
    // Continue anyway - RmlUi has fallback fonts
  }

  m_initialized = true;
  return true;
}

void
UIManager::shutdown()
{
  if (!m_initialized) {
    return;
  }

  // Unload all documents
  for (auto& [name, info] : m_documents) {
    if (info.document) {
      info.document->Close();
    }
  }
  m_documents.clear();

  // Destroy context
  if (m_context) {
    Rml::RemoveContext(m_context->GetName());
    m_context = nullptr;
  }

  // Shutdown RmlUi
  Rml::Shutdown();

  m_renderInterface.reset();
  m_systemInterface.reset();

  m_initialized = false;
}

void
UIManager::update(float dt)
{
  if (!m_initialized || !m_context) {
    return;
  }

  (void)dt; // Unused for now, but kept for future frame-based animations
  m_context->Update();
}

void
UIManager::render()
{
  if (!m_initialized || !m_context) {
    return;
  }

#ifndef NDEBUG
  // Clear any existing OpenGL errors before UI rendering
  while (glGetError() != GL_NO_ERROR) {
  }
#endif

  // Save OpenGL state
  GLboolean blendEnabled = glIsEnabled(GL_BLEND);
  GLboolean depthTestEnabled = glIsEnabled(GL_DEPTH_TEST);
  GLboolean cullFaceEnabled = glIsEnabled(GL_CULL_FACE);
  GLboolean scissorTestEnabled = glIsEnabled(GL_SCISSOR_TEST);

  // Set up state for UI rendering
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glDisable(GL_DEPTH_TEST);
  glDisable(GL_CULL_FACE);

  // Get the renderer and call GL3-specific begin/end frame
  auto* renderer = static_cast<RenderInterface_GL3*>(m_renderInterface.get());
  renderer->BeginFrame();
  m_context->Render();
  renderer->EndFrame();

#ifndef NDEBUG
  // Clear any OpenGL errors from RmlUi's advanced features
  // (framebuffer operations that fail gracefully)
  while (glGetError() != GL_NO_ERROR) {
  }
#endif

  // Restore OpenGL state
  if (!blendEnabled) {
    glDisable(GL_BLEND);
  }
  if (depthTestEnabled) {
    glEnable(GL_DEPTH_TEST);
  }
  if (cullFaceEnabled) {
    glEnable(GL_CULL_FACE);
  }
  if (!scissorTestEnabled) {
    glDisable(GL_SCISSOR_TEST);
  }
}

bool
UIManager::loadDocument(const std::string& path, const std::string& name)
{
  if (!m_initialized || !m_context) {
    std::cerr << "UIManager not initialized\n";
    return false;
  }

  // Check if document already loaded
  if (m_documents.find(name) != m_documents.end()) {
    return true; // Already loaded is a success case
  }

  // Construct full path
  std::string fullPath = "resources/ui/" + path;

  // Load document
  Rml::ElementDocument* document = m_context->LoadDocument(fullPath);
  if (!document) {
    std::cerr << "Failed to load document: " << fullPath << std::endl;
    return false;
  }

  // Store document info
  m_documents[name] = { .document = document, .path = path };
  return true;
}

void
UIManager::showDocument(const std::string& name)
{
  auto it = m_documents.find(name);
  if (it == m_documents.end()) {
    std::cerr << "Document '" << name << "' not found\n";
    return;
  }

  if (it->second.document) {
    it->second.document->Show();
  }
}

void
UIManager::hideDocument(const std::string& name)
{
  auto it = m_documents.find(name);
  if (it == m_documents.end()) {
    std::cerr << "Document '" << name << "' not found\n";
    return;
  }

  if (it->second.document) {
    // Clear focus from any elements in this document before hiding
    Rml::Element* focus = m_context->GetFocusElement();
    if (focus && focus->GetOwnerDocument() == it->second.document) {
      focus->Blur();
    }

    it->second.document->Hide();
  }
}

void
UIManager::unloadDocument(const std::string& name)
{
  auto it = m_documents.find(name);
  if (it == m_documents.end()) {
    return;
  }

  if (it->second.document) {
    it->second.document->Close();
  }

  m_documents.erase(it);
}

bool
UIManager::isDocumentVisible(const std::string& name) const
{
  auto it = m_documents.find(name);
  if (it == m_documents.end()) {
    return false;
  }

  return it->second.document && it->second.document->IsVisible();
}

void
UIManager::onResize(int width, int height)
{
  if (!m_initialized || !m_context) {
    return;
  }

  m_width = width;
  m_height = height;
  m_context->SetDimensions(Rml::Vector2i(width, height));

  // Update renderer viewport
  auto* renderer = static_cast<RenderInterface_GL3*>(m_renderInterface.get());
  renderer->SetViewport(width, height);
}

void
UIManager::onMouseMove(int x, int y, int modifiers)
{
  if (!m_initialized || !m_context) {
    return;
  }

  m_context->ProcessMouseMove(x, y, modifiers);
}

void
UIManager::onMouseButton(int button, int action, int modifiers)
{
  if (!m_initialized || !m_context) {
    return;
  }

  if (action == GLFW_PRESS) {
    m_context->ProcessMouseButtonDown(button, modifiers);
  } else if (action == GLFW_RELEASE) {
    m_context->ProcessMouseButtonUp(button, modifiers);
  }
}

void
UIManager::onMouseScroll(double yoffset, int modifiers)
{
  if (!m_initialized || !m_context) {
    return;
  }

  m_context->ProcessMouseWheel(static_cast<float>(-yoffset), modifiers);
}

void
UIManager::onKey(int key, int action, int modifiers)
{
  if (!m_initialized || !m_context) {
    return;
  }

  // Convert GLFW key to RmlUi key
  Rml::Input::KeyIdentifier rmlKey = RmlGLFW::ConvertKey(key);

  if (action == GLFW_PRESS || action == GLFW_REPEAT) {
    m_context->ProcessKeyDown(rmlKey, modifiers);
  } else if (action == GLFW_RELEASE) {
    m_context->ProcessKeyUp(rmlKey, modifiers);
  }
}

void
UIManager::onTextInput(unsigned int codepoint)
{
  if (!m_initialized || !m_context) {
    return;
  }

  m_context->ProcessTextInput(codepoint);
}

bool
UIManager::wantsMouseCapture() const
{
  if (!m_initialized || !m_context) {
    return false;
  }

  // Check if mouse is over any UI element
  return m_context->IsMouseInteracting();
}

bool
UIManager::wantsKeyboardCapture() const
{
  if (!m_initialized || !m_context) {
    return false;
  }

  // Check if any text input element has focus
  Rml::Element* focus = m_context->GetFocusElement();
  if (!focus) {
    return false;
  }

  // Check if the focused element is in a visible document
  Rml::ElementDocument* doc = focus->GetOwnerDocument();
  if (!doc) {
    return false;
  }

  // Only capture keyboard if the document is visible
  return doc->IsVisible();
}

bool
UIManager::registerClickCallback(const std::string& documentName,
                                 const std::string& elementId,
                                 ClickCallback callback)
{
  if (!m_initialized || !m_context) {
    std::cerr << "UIManager not initialized\n";
    return false;
  }

  // Find the document
  auto it = m_documents.find(documentName);
  if (it == m_documents.end()) {
    std::cerr << "Document '" << documentName << "' not found\n";
    return false;
  }

  // Find the element by ID
  Rml::Element* element = it->second.document->GetElementById(elementId);
  if (!element) {
    std::cerr << "Element '" << elementId << "' not found in document '"
              << documentName << "'\n";
    return false;
  }

  // Create event listener
  auto listener = std::make_unique<UIEventListener>(callback);

  // Attach the listener to the element's click event
  element->AddEventListener(Rml::EventId::Click, listener.get());

  // Store the listener to keep it alive
  m_eventListeners.push_back(std::move(listener));

  std::cout << "Registered click callback for element '" << elementId
            << "' in document '" << documentName << "'\n";
  return true;
}
