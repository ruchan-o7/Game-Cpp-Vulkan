#include "Application.h"
#include "../ImGui/ImGuiLayer.h"
#include "../Core/AssetManager.h"
#include "../Core/Time.h"
#include "../Events/Event.h"
#include "../Profile/Profiling.h"
#include "../Core/Log.h"
#include "../Events/KeyEvent.h"
#include "../Events/MouseEvent.h"
#include "../Renderer/Renderer.h"
#include "../Renderer/VulkanGraphicsPipeline.h"
#include "../Renderer/VulkanShader.h"

#include <mutex>
#include "GLFW/glfw3.h"
#include <imgui.h>
namespace FooGame {

Application* Application::s_Instance = nullptr;

static void GLFWErrorCallback(int err, const char* desc) {
  FOO_CORE_ERROR("GLFW Error ({0}): {1}", err, desc);
}

Application::Application(const ApplicationSpecifications& spec) : m_Specs(spec) {
  FOO_PROFILE_FUNCTION();
  FOO_ASSERT(!s_Instance, "Application already exists!");
  s_Instance = this;

  if (!m_Specs.WorkingDirectory.empty()) {
    std::filesystem::current_path(m_Specs.WorkingDirectory);
  }

  if (!glfwInit()) {
    const char* msg = nullptr;
    glfwGetError(&msg);
    FOO_ENGINE_ERROR("Can not initialize glfw! Err: {}, Terminating!", msg);
    Close();
  }

  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

  m_Window = glfwCreateWindow(1600, 900, spec.Name.c_str(), nullptr, nullptr);

  glfwSetWindowUserPointer(m_Window, this);
  glfwSetErrorCallback(GLFWErrorCallback);
  if (!glfwVulkanSupported()) {
    FOO_ENGINE_CRITICAL("Vulkan not supported!");
    return;
  }

  m_Renderer = fg::Renderer::Create(m_Window, nullptr);
  {
    fg::ShaderDescription shaderDesc;
    shaderDesc.EntryPoint = "main";
    shaderDesc.Path = "Assets/Shaders/triangle_vert.spv";
    shaderDesc.Stage = VK_SHADER_STAGE_VERTEX_BIT;
    auto triVert = m_Renderer->CreateShader(shaderDesc);

    shaderDesc.Path = "Assets/Shaders/triangle_frag.spv";
    shaderDesc.Stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    auto trifrag = m_Renderer->CreateShader(shaderDesc);

    fg::GraphicsPipelineDescription pipeDesc;
    pipeDesc.FragmentShader = trifrag.get();
    pipeDesc.VertexShader = triVert.get();
    pipeDesc.DynamicStates = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};

    m_Pipeline = m_Renderer->CreateGraphicsPipeline(pipeDesc);
  }
  m_Renderer->BeginRendering();
  m_Renderer->EndRendering();
  m_Swapchain = m_Renderer->GetSwapchain();

  // AssetManager::Init();

  // m_ImGuiLayer = new ImGuiLayer;
  // PushLayer(m_ImGuiLayer);

  glfwSetWindowSizeCallback(m_Window, [](GLFWwindow* window, int w, int h) {
    Application& data = *(Application*)glfwGetWindowUserPointer(window);
    WindowResizeEvent e(w, h);
    data.OnEvent(e);
  });
  glfwSetWindowCloseCallback(m_Window, [](GLFWwindow* window) {
    Application& data = *(Application*)glfwGetWindowUserPointer(window);
    WindowCloseEvent e;
    data.OnEvent(e);
  });
  glfwSetKeyCallback(m_Window, [](GLFWwindow* window, int key, int scancode, int action, int mods) {
    Application& data = *(Application*)glfwGetWindowUserPointer(window);
    switch (action) {
      case GLFW_PRESS: {
        KeyPressedEvent event(key, 0);
        data.OnEvent(event);
        break;
      }
      case GLFW_RELEASE: {
        KeyReleasedEvent event(key);
        data.OnEvent(event);
        break;
      }
      case GLFW_REPEAT: {
        KeyPressedEvent event(key, true);
        data.OnEvent(event);
        break;
      }
    }
  });
  glfwSetCharCallback(m_Window, [](GLFWwindow* window, unsigned int keycode) {
    Application& data = *(Application*)glfwGetWindowUserPointer(window);
    KeyTypedEvent e(keycode);
    data.OnEvent(e);
  });

  glfwSetFramebufferSizeCallback(m_Window, [](GLFWwindow* window, int width, int height) {
    Application& data = *(Application*)glfwGetWindowUserPointer(window);
    WindowResizeEvent e {static_cast<unsigned int>(width), static_cast<unsigned int>(height)};
    data.OnEvent(e);
  });
  glfwSetMouseButtonCallback(m_Window, [](GLFWwindow* window, int button, int action, int mods) {
    Application& data = *(Application*)glfwGetWindowUserPointer(window);
    switch (action) {
      case GLFW_PRESS: {
        MouseButtonPressedEvent e {static_cast<MouseCode>(button)};
        data.OnEvent(e);
        break;
      }
      case GLFW_RELEASE: {
        MouseButtonReleasedEvent e {static_cast<MouseCode>(button)};
        data.OnEvent(e);
        break;
      }
    }
  });
  glfwSetCursorPosCallback(m_Window, [](GLFWwindow* window, double xPos, double yPos) {
    Application& data = *(Application*)glfwGetWindowUserPointer(window);
    MouseMovedEvent e {static_cast<float>(xPos), static_cast<float>(yPos)};
    data.OnEvent(e);
  });
  glfwSetScrollCallback(m_Window, [](GLFWwindow* window, double xOffset, double yOffset) {
    Application& data = *(Application*)glfwGetWindowUserPointer(window);
    MouseScrolledEvent e {static_cast<float>(xOffset), static_cast<float>(yOffset)};
    data.OnEvent(e);
  });
}
Application::~Application() {
  AssetManager::DeInit();
  m_Renderer->Destroy();
}
void Application::PushLayer(Layer* layer) {
  m_LayerStack.PushOverlay(layer);
  layer->OnAttach();
}
void Application::Close() {
  m_Running = false;
}

void Application::SubmitToMainThread(const std::function<void()>& f) {
  std::scoped_lock<std::mutex> lock(m_MainThreadQueueMutex);
  m_MainThreadQueue.emplace_back(f);
}
void Application::OnEvent(Event& e) {
  EventDispatcher dispatcher {e};
  dispatcher.Dispatch<WindowCloseEvent>(BIND_EVENT_FN(Application::OnWindowClose));
  dispatcher.Dispatch<WindowResizeEvent>(BIND_EVENT_FN(Application::OnWindowResize));
  for (auto it = m_LayerStack.begin(); it != m_LayerStack.end(); ++it) {
    if (e.Handled) {
      break;
    }
    (*it)->OnEvent(e);
  }
}
void Application::Run() {
  while (m_Running) {
    Time::UpdateCurrentTime();
    auto time = Time::CurrentTime();
    float ts = time - m_LastFrameTime;
    m_LastFrameTime = time;
    ExecuteMainThreadQueue();

    if (!m_Minimized) {
      // m_ImGuiLayer->Begin(&m_MenuBarCallback);
      // for (Layer* l : m_LayerStack) {
      //  l->OnUpdate(ts);
      //}
      // auto stats = Renderer3D::GetStats();
      //
      // Renderer3D::EndDraw();
      // ImGui::Begin("3d scene stats");
      // ImGui::Text("Draw calls %i", stats.DrawCall);
      // ImGui::Text("Vertex count %llu", stats.VertexCount);
      // ImGui::Text("Index count %llu", stats.IndexCount);
      // ImGui::End();

      // for (Layer* l : m_LayerStack) {
      //   l->OnImGuiRender();
      // }
      // m_ImGuiLayer->End();
      m_Renderer->BeginRendering();
      m_Renderer->BindPipeline(m_Pipeline);
      m_Renderer->Draw({3, 1, 0, 0});

      m_Renderer->EndRendering();

      m_Swapchain->Present();
    }
    glfwPollEvents();
  }
}
bool Application::OnWindowClose(WindowCloseEvent& e) {
  m_Running = false;
  return true;
}
bool Application::OnWindowResize(WindowResizeEvent& e) {
  if (e.GetWidth() == 0 || e.GetHeight() == 0) {
    m_Minimized = true;
    return false;
  }
  m_Minimized = false;
  return false;
  // return Backend::OnWindowResized(e);
}
void Application::ExecuteMainThreadQueue() {
  std::scoped_lock<std::mutex> lock(m_MainThreadQueueMutex);
  for (auto& f : m_MainThreadQueue) {
    f();
  }
  m_MainThreadQueue.clear();
}
void Application::SetMenubarCallback(const std::function<void()>& callback) {
  m_MenuBarCallback = callback;
}
}  // namespace FooGame
