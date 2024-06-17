#include "CameraController.h"
#include "../Scene/Component.h"
#include "../Engine/Camera/Camera.h"
#include <Core.h>
#include <imgui.h>
namespace FooGame
{
    void CameraController::OnUpdate(float ts)
    {
        if (Input::IsKeyDown(KeyCode::W))
        {
            m_pCamera->MoveUp();
        }
        if (Input::IsKeyDown(KeyCode::S))
        {
            m_pCamera->MoveDown();
        }
        if (Input::IsKeyDown(KeyCode::D))
        {
            m_pCamera->MoveRight();
        }
        if (Input::IsKeyDown(KeyCode::A))
        {
            m_pCamera->MoveLeft();
        }
        m_pCamera->Update(ts);
        ImGui::Begin("Camera");
        auto cameraPos = m_pCamera->Position;
        float pos[3]   = {cameraPos.x, cameraPos.y, cameraPos.z};
        float fov      = m_pCamera->Fov;
        float rot[3]   = {
            m_pCamera->Rotation.x,
            m_pCamera->Rotation.y,
            m_pCamera->Rotation.z,
        };
        float front[3] = {
            m_pCamera->Front.x,
            m_pCamera->Front.y,
            m_pCamera->Front.z,
        };

        ImGui::DragFloat("Movement Speed", &m_pCamera->MovementSpeed, 0.1f, 0.1f, 5.0f);
        ImGui::DragFloat("Rotation Speed", &m_pCamera->RotationSpeed, 0.1f, 0.1f, 5.0f);
        ImGui::Checkbox("Flip y", &m_pCamera->flipY);
        ImGui::DragFloat("Fov", &fov, 0.1f, 0.1f, 179.0f);
        ImGui::DragFloat3("Position", pos, 0.1f, -1000.0f, 1000.0f);
        ImGui::DragFloat3("Rotation", rot, 0.1f, -1000.0f, 1000.0f);
        ImGui::DragFloat("ZNear", &m_pCamera->ZNear, 0.01f, 0.0f, 1000000.0f);
        ImGui::DragFloat("ZFar", &m_pCamera->ZFar, 0.01f, 0.0f, 1000000.0f);
        ImGui::End();

        m_pCamera->SetPosition(glm::vec3{pos[0], pos[1], pos[2]});
        m_pCamera->SetRotation(glm::vec3(rot[0], rot[1], rot[2]));
        m_pCamera->SetFov(fov);
    }
    void CameraController::OnCreate()
    {
        m_pCamera    = GetComponent<CameraComponent>().pCamera;
        m_pTransform = &GetComponent<TransformComponent>();
        m_pCamera->setPerspective(60.0f, (float)1600.0f / (float)900.0f, 0.1f, 512.0f);
        m_pCamera->SetRotation(glm::vec3(-12.0f, 159.0f, 0.0f));
        m_pCamera->SetTranslatin(glm::vec3(0.0f, 0.5f, 0.5f));
        m_pCamera->MovementSpeed = 0.1f;
        m_pCamera->Fov           = 90.f;
        m_pCamera->type          = Camera::CameraType::firstperson;
    }
}  // namespace FooGame