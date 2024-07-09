#include "ImGuiLayer.h"
#include "imgui.h"
#include <vulkan/vulkan.h>
#include "../Engine/Engine/Backend.h"
#include "../Core/Application.h"
#include <imgui.h>
#include <imgui_internal.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_vulkan.h>
namespace FooGame
{
    static VkDescriptorPool g_ImguiPool = nullptr;

    static void check_vk_result(VkResult err)
    {
        if (err == 0)
        {
            return;
        }
        fprintf(stderr, "[vulkan] Error: VkResult = %d\n", err);
        if (err < 0)
        {
            abort();
        }
    }
    ImGuiLayer::ImGuiLayer() : Layer("ImGuiLayer")
    {
    }
    void ImGuiLayer::OnAttach()
    {
        auto pRenderDevice                = Backend::GetRenderDevice();
        auto device                       = pRenderDevice->GetVkDevice();
        VkDescriptorPoolSize pool_sizes[] = {
            {               VK_DESCRIPTOR_TYPE_SAMPLER, 1000},
            {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000},
            {         VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000},
            {         VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000},
            {  VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000},
            {  VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000},
            {        VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000},
            {        VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000},
            {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000},
            {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000},
            {      VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000}
        };

        VkDescriptorPoolCreateInfo pool_info = {};
        pool_info.sType                      = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        pool_info.flags                      = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
        pool_info.maxSets                    = 1000;
        pool_info.poolSizeCount              = ARRAY_COUNT(pool_sizes);
        pool_info.pPoolSizes                 = pool_sizes;

        auto res = vkCreateDescriptorPool(device, &pool_info, nullptr, &g_ImguiPool);
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO();
        (void)io;
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;  // Enable Keyboard Controls
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;   // Enable Gamepad Controls
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
        io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

        ImGui::StyleColorsDark();

        auto& style = ImGui::GetStyle();
        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
        {
            style.WindowRounding              = 0.2f;
            style.Colors[ImGuiCol_WindowBg].w = 1.0f;
        }

        Application& app = Application::Get();

        ImGui_ImplGlfw_InitForVulkan(app.GetWindow().GetWindowHandle(), true);

        auto queueProps = pRenderDevice->GetPhysicalDevice()->GetQueueProperties();

        auto graphics = pRenderDevice->GetPhysicalDevice()->FindQueueFamily(VK_QUEUE_GRAPHICS_BIT);

        ImGui_ImplVulkan_InitInfo init_info = {};
        init_info.Instance                  = pRenderDevice->GetVkInstance();
        init_info.PhysicalDevice            = pRenderDevice->GetVkPhysicalDevice();
        init_info.Device                    = pRenderDevice->GetVkDevice();
        init_info.Queue           = pRenderDevice->GetLogicalDevice()->GetQueue(0, graphics);
        init_info.QueueFamily     = graphics;
        init_info.DescriptorPool  = g_ImguiPool;
        init_info.RenderPass      = Backend::GetRenderPass();
        init_info.Subpass         = 0;
        init_info.MinImageCount   = 2;
        init_info.ImageCount      = 2;
        init_info.CheckVkResultFn = check_vk_result;
        init_info.MSAASamples     = VK_SAMPLE_COUNT_1_BIT;

        ImGui_ImplVulkan_Init(&init_info);
        ImGui_ImplVulkan_CreateFontsTexture();
    }
    void ImGuiLayer::OnDetach()
    {
        auto device = Backend::GetRenderDevice()->GetVkDevice();
        ImGui_ImplVulkan_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();

        vkDestroyDescriptorPool(device, g_ImguiPool, nullptr);
    }
    void ImGuiLayer::OnEvent(Event& e)
    {
        if (m_BlockEvents)
        {
            ImGuiIO& io  = ImGui::GetIO();
            e.Handled   |= e.IsInCategory(EventCategoryMouse) & io.WantCaptureMouse;
            e.Handled   |= e.IsInCategory(EventCategoryKeyboard) & io.WantCaptureKeyboard;
        }
    }
    void ImGuiLayer::Begin(std::function<void()>* callback)
    {
        ImGui_ImplVulkan_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        {
            static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;
            ImGuiWindowFlags window_flags             = ImGuiWindowFlags_NoDocking;
            if (callback)
            {
                window_flags |= ImGuiWindowFlags_MenuBar;
            }
            const auto* viewport = ImGui::GetMainViewport();
            ImGui::SetNextWindowPos(viewport->WorkPos);
            ImGui::SetNextWindowSize(viewport->Size);

            ImGui::SetNextWindowViewport(viewport->ID);
            ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
            ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
            window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse |
                            ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
            window_flags    |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
            dockspace_flags |= ImGuiDockNodeFlags_PassthruCentralNode;
            // When using ImGuiDockNodeFlags_PassthruCentralNode, DockSpace() will render our
            // background and handle the pass-thru hole, so we ask Begin() to not render a
            // background.
            if (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode)
            {
                window_flags |= ImGuiWindowFlags_NoBackground;
            }

            // Important: note that we proceed even if Begin() returns false (aka window is
            // collapsed). This is because we want to keep our DockSpace() active. If a DockSpace()
            // is inactive, all active windows docked into it will lose their parent and become
            // undocked. We cannot preserve the docking relationship between an active window and an
            // inactive docking, otherwise any change of dockspace/settings would lead to windows
            // being stuck in limbo and never being visible.
            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
            ImGui::Begin("DockSpace Demo", nullptr, window_flags);
            ImGui::PopStyleVar();

            ImGui::PopStyleVar(2);

            // Submit the DockSpace
            ImGuiIO& io = ImGui::GetIO();
            if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
            {
                ImGuiID dockspace_id = ImGui::GetID("VulkanAppDockspace");
                ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
            }

            if (callback)
            {
                if (ImGui::BeginMenuBar())
                {
                    (*callback)();
                    ImGui::EndMenuBar();
                }
            }
        }
    }
    void ImGuiLayer::End()
    {
        ImGui::End();  // End for 'WorkSpace Demo'
        auto cb          = Backend::GetCurrentCommandbuffer();
        ImGuiIO& io      = ImGui::GetIO();
        Application& app = Application::Get();
        io.DisplaySize   = ImVec2(app.GetWindow().GetWidth(), app.GetWindow().GetHeight());

        ImGui::Render();
        ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), cb);

        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
        {
            ImGui::UpdatePlatformWindows();
            ImGui::RenderPlatformWindowsDefault();
        }
    }
}  // namespace FooGame
