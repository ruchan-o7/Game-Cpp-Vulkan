#include "CameraController.h"
#include "../Scene/Component.h"
#include "../Engine/Camera/Camera.h"
#include <imgui.h>
#include "../Input/Input.h"
#include "../Core/Window.h"

namespace FooGame::Script
{
    void CameraController::OnUpdate(float ts)
    {
        auto Rotation = m_pTransform->Rotation;
        auto Front    = m_pCamera->Front;
        if (Input::IsKeyDown(Key::W))
        {
            m_pTransform->Translation += Front * MovementSpeed;
        }
        if (Input::IsKeyDown(Key::S))
        {
            m_pTransform->Translation -= Front * MovementSpeed;
        }
        if (Input::IsKeyDown(Key::D))
        {
            m_pTransform->Translation +=
                glm::normalize(glm::cross(Front, glm::vec3(0.0f, 1.0f, 0.0f))) * MovementSpeed;
        }
        if (Input::IsKeyDown(Key::A))
        {
            m_pTransform->Translation -=
                glm::normalize(glm::cross(Front, glm::vec3(0.0f, 1.0f, 0.0f))) * MovementSpeed;
        }
        m_pCamera->Position = m_pTransform->Translation;

        ImGui::Begin("Camera");
        float frontArray[3] = {
            Front.x,
            Front.y,
            Front.z,
        };
        float rotArr[3] = {
            m_pTransform->Rotation.x,
            m_pTransform->Rotation.y,
            m_pTransform->Rotation.z,
        };

        ImGui::DragFloat3("Rotation", rotArr, 0.1f, -10000.f, 10000.0f);
        ImGui::DragFloat3("Front Vector", frontArray, 0.1f, -10000.f, 10000.0f);
        ImGui::DragFloat("Movement Speed", &MovementSpeed, 0.1f, 0.1f, 15.0f);
        ImGui::DragFloat("Rotation Speed", &RotationSpeed, 0.1f, 0.1f, 15.0f);
        ImGui::Checkbox("Flip y", &FlipY);
        ImGui::DragFloat("Fov", &m_pCamera->Fov, 0.1f, 0.1f, 179.0f);
        ImGui::End();
        m_pTransform->Rotation[0] = rotArr[0];
        m_pTransform->Rotation[1] = rotArr[1];
        m_pTransform->Rotation[2] = rotArr[2];

        m_pCamera->Rotation = m_pTransform->Rotation;

        auto& window      = Window::Get();
        m_pCamera->Aspect = (float)window.GetWidth() / (float)window.GetHeight();
        m_pCamera->UpdateMatrix();
    }
    void CameraController::OnCreate()
    {
        m_pCamera           = GetComponent<CameraComponent>().pCamera;
        m_pTransform        = &GetComponent<TransformComponent>();
        m_pCamera->Position = m_pTransform->Translation;
        m_pCamera->Rotation = m_pTransform->Rotation;
        m_pCamera->type     = Camera::Type::FirstPerson;
    }
}  // namespace FooGame::Script
