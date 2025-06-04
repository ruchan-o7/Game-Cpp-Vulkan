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
#include "Buffer.h"

#include <mutex>
#include "GLFW/glfw3.h"
#include "glm/ext/matrix_clip_space.hpp"
#include "glm/ext/matrix_transform.hpp"
#include "src/Renderer/VulkanBuffer.h"
#include "src/Renderer/VulkanImage.h"
#include <imgui.h>
#include <stb_image.h>
namespace FooGame {

Application* Application::s_Instance = nullptr;

static void GLFWErrorCallback(int err, const char* desc) {
  FOO_CORE_ERROR("GLFW Error ({0}): {1}", err, desc);
}

fg::Ref<fg::VulkanBuffer> m_VertexBuffer;
fg::Ref<fg::VulkanBuffer> m_UniformBuffer;
fg::Ref<fg::VulkanImage> m_Texture;
VkDescriptorSet m_DescriptorSet = 0;

struct UBO {
    glm::mat4 Model;
    glm::mat4 View;
    glm::mat4 Proj;
};

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
  m_Renderer->CreateDeviceAndSwapchain();
  m_Swapchain = m_Renderer->GetSwapchain();
  {
    int width = 0, height = 0, channel = 0;
    fg::Buffer pixelData;
    stbi_uc* pixels =
        stbi_load("Assets/textures/texture.jpg", &width, &height, &channel, STBI_rgb_alpha);
    pixelData.Data = pixels;

    pixelData.Size = width * height * channel;

    fg::ImageDescription imageDesc;
    imageDesc.Format = fg::ImageFormat::RGBA8;

    imageDesc.Width = width;
    imageDesc.Height = height;
    imageDesc.Depth = 1;
    imageDesc.MipLevels = 1;
    imageDesc.Usage = fg::ImageUsage::ShaderResource;
    imageDesc.Type = fg::ImageType::Type2D;
    imageDesc.Name = "Texture";

    m_Texture = m_Renderer->CreateImage(imageDesc, pixelData);
  }
  {
    fg::BufferDescription desc;
    desc.Size = sizeof(UBO);
    desc.Name = "Uniform buffer";
    desc.Usage = fg::BufferUsage::Uniform;
    m_UniformBuffer = m_Renderer->CreateBuffer(desc);
  }
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
    pipeDesc.RenderTargetFormat = m_Swapchain->Format().format;
    pipeDesc.VertexAttributes = {
        {0, 0, fg::VT_VEC2},
        {0, 1, fg::VT_VEC3},
        {0, 2, fg::VT_VEC2},
    };
    pipeDesc.BindingDescs = {
        {0, sizeof(glm::vec2) + sizeof(glm::vec3)}
    };
    pipeDesc.ShaderVariables = {
        {0, 0,         VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,   VK_SHADER_STAGE_VERTEX_BIT},
        {1, 0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT},
    };

    m_Pipeline = m_Renderer->CreateGraphicsPipeline(pipeDesc);
    m_DescriptorSet = m_Pipeline->CreateDescriptorSet();

    VkDescriptorBufferInfo uboInfo {};
    uboInfo.buffer = m_UniformBuffer->GetVkBuffer();
    uboInfo.offset = 0;
    uboInfo.range = VK_WHOLE_SIZE;

    VkDescriptorImageInfo imageInfo {};
    imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    imageInfo.imageView = m_Texture->GetDefaultView()->GetHandle();
    imageInfo.sampler = m_Pipeline->GetSampler();

    std::array<VkWriteDescriptorSet, 2> wds {};
    wds[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    wds[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;

    wds[0].dstSet = m_DescriptorSet;
    wds[0].dstBinding = 0;
    wds[0].dstArrayElement = 0;
    wds[0].pBufferInfo = &uboInfo;
    wds[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    wds[0].descriptorCount = 1;
    wds[0].pBufferInfo = &uboInfo;

    wds[1].dstSet = m_DescriptorSet;
    wds[1].dstBinding = 1;
    wds[1].dstArrayElement = 0;
    wds[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    wds[1].descriptorCount = 1;
    wds[1].pImageInfo = &imageInfo;

    m_Renderer->GetLogicalDevice()->UpdateDescriptorSets(wds.size(), wds.data(), 0, nullptr);

    struct Vertex {
        glm::vec2 pos;
        glm::vec3 color;
        glm::vec2 texCoord;
    };

    constexpr std::array<Vertex, 3> vertices = {
        Vertex {{0.0f, -0.5f}, {1.0f, 1.0f, 1.0f}, {1.0f, 0.0f}},
        Vertex { {0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}},
        Vertex {{-0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}}
    };
    fg::BufferDescription desc;
    desc.Size = vertices.size() * sizeof(Vertex);
    desc.Name = "Vertex buffer";
    desc.Usage = fg::BufferUsage::Vertex;

    fg::Buffer data;
    data.Data = (uint8_t*)vertices.data();
    data.Size = desc.Size;
    m_VertexBuffer = m_Renderer->CreateBuffer(desc, data);
  }

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
  m_Texture->Release();
  m_VertexBuffer->Release();
  m_UniformBuffer->Release();
  m_Pipeline->Release();
  m_Swapchain.reset();
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
      UBO ubo {};
      ubo.Model =
          glm::rotate(glm::mat4(1.f), (float)time * glm::radians(90.f), glm::vec3(0.f, 0.f, 1.0f));
      ubo.View =
          glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f), glm::vec3(0.f, 0.f, 1.0f));
      ubo.Proj = glm::perspective(
          glm::radians(45.f),
          (float)m_Swapchain->GetExtent().width / m_Swapchain->GetExtent().height, 0.001f, 100.f);
      ubo.Proj[1][1] *= -1;
      uint8_t CHUNK_BOI[192];
      fg::Buffer mvp;
      mvp.Data = CHUNK_BOI;
      std::memcpy(mvp.Data, &ubo, sizeof(ubo));
      mvp.Size = sizeof(ubo);
      m_UniformBuffer->SetData(mvp);

      fg::VulkanBuffer* buffer[1] = {m_VertexBuffer.get()};
      VkDeviceSize offset[] = {0};
      m_Renderer->BindDescriptorSet(m_DescriptorSet);
      m_Renderer->BindVertexBuffers(0, 1, buffer, offset);
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
