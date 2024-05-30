#include <vector>
#include <Engine.h>
#include "EditorLayer.h"
#include "EditorSceneDeserializer.h"
#include "glm/fwd.hpp"
#include "imgui.h"
#include "src/Input/KeyCodes.h"
#include <Log.h>
namespace FooGame {

#define SCENE_JSON "Assets/Scenes/Prototype/scene.json"
  EditorLayer::EditorLayer(const CommandLineArgs& args)
	  : Layer("Editor Layer"), m_Args(args) {
	FOO_EDITOR_INFO("Editor layer Created");
  }
  void EditorLayer::OnAttach() {
	FOO_EDITOR_INFO("Reading scene data");
#ifdef FOO_DEBUG
	EditorSceneDeserializer serializer;
	m_EditorScene = std::move(serializer.DeSerialize(SCENE_JSON));
#else
	if (m_Args.count > 1) {
	  std::cout << m_Args.argv[1] << std::endl;
	  std::ifstream f {m_Args.argv[1]};
	  json          data = json::parse(f);
	  f.close();
	}

#endif
	size_t vertexSize = 0;
	size_t indexSize  = 0;
	FOO_EDITOR_INFO("Scene : {0}", m_EditorScene->Name);
	FOO_EDITOR_INFO("Textures size : {0}", m_EditorScene->Textures.size());
	FOO_EDITOR_INFO("Mesh size : {0}", m_EditorScene->MeshDatas.size());
	for (int i = 0; i < m_EditorScene->MeshDatas.size(); i++) {
	  auto& mData = m_EditorScene->MeshDatas[i];
	  for (const auto& mesh : mData.ModelPtr->GetMeshes()) {
		vertexSize += mesh.m_Vertices.size() * sizeof(Vertex);
		indexSize  += mesh.m_Indices.size() * sizeof(uint32_t);
	  }
	}
	FOO_EDITOR_INFO("Will allocate {0} of kbytes for vertices",
					(double)vertexSize / 1024.0);
	FOO_EDITOR_INFO("Will allocate {0} of kbytes for indices",
					(double)indexSize / 1024.0);

	for (auto& meshData : m_EditorScene->MeshDatas) {
	  Renderer3D::SubmitModel(meshData.ModelPtr.get());
	}
	m_Camera2.setPerspective(60.0f, (float)1600.0f / (float)900.0f, 0.1f,
							 512.0f);
	m_Camera2.SetRotation(glm::vec3(-12.0f, 159.0f, 0.0f));
	m_Camera2.SetTranslatin(glm::vec3(0.0f, 0.5f, 0.5f));
	m_Camera2.MovementSpeed = 0.1f;
	m_Camera2.Fov           = 90.f;
	m_Camera2.type          = Camera::CameraType::firstperson;
  }
  void EditorLayer::OnDetach() { Renderer3D::ClearBuffers(); }
  void EditorLayer::OnImGuiRender() {
	for (int i = 0; i < m_EditorScene->MeshDatas.size(); i++) {
	  auto& mD = m_EditorScene->MeshDatas[i];
	  if (ImGui::TreeNode("Models", "%s", mD.ModelPtr->Name.c_str())) {
		ImGui::PushID(i);
		if (ImGui::TreeNode("Position")) {
		  float pos[3] = {
			  mD.Transform.Translation.x,
			  mD.Transform.Translation.y,
			  mD.Transform.Translation.z,
		  };
		  ImGui::DragFloat3("Position", pos, 0.005, -FLT_MAX, +FLT_MAX);
		  ImGui::TreePop();
		  mD.Transform.Translation = glm::vec3 {
			  pos[0],
			  pos[1],
			  pos[2],
		  };
		}
		if (ImGui::TreeNode("Scale")) {
		  float scale[3] = {
			  mD.Transform.Scale.x,
			  mD.Transform.Scale.y,
			  mD.Transform.Scale.z,
		  };

		  ImGui::DragFloat3("Scale", scale, 0.005, -FLT_MAX, +FLT_MAX);
		  ImGui::TreePop();
		  mD.Transform.Scale = glm::vec3 {
			  scale[0],
			  scale[1],
			  scale[2],
		  };
		}
		if (ImGui::TreeNode("Rotation")) {
		  float rot[3] = {
			  mD.Transform.Rotation.x,
			  mD.Transform.Rotation.y,
			  mD.Transform.Rotation.z,
		  };
		  ImGui::DragFloat3("Rotation", rot, 0.005, -FLT_MAX, +FLT_MAX);
		  ImGui::TreePop();
		  mD.Transform.Rotation = glm::vec3 {
			  rot[0],
			  rot[1],
			  rot[2],
		  };
		}
		ImGui::PopID();
		ImGui::TreePop();
	  }
	}
	{
	  ImGui::Begin("Camera");
	  auto  cameraPos = m_Camera2.Position;
	  float pos[3]    = {cameraPos.x, cameraPos.y, cameraPos.z};
	  float fov       = m_Camera2.Fov;
	  float rot[3]    = {
          m_Camera2.Rotation.x,
          m_Camera2.Rotation.y,
          m_Camera2.Rotation.z,
      };
	  float front[3] = {
		  m_Camera2.Front.x,
		  m_Camera2.Front.y,
		  m_Camera2.Front.z,
	  };

	  ImGui::DragFloat("Movement Speed", &m_Camera2.MovementSpeed, 0.1f, 0.1f,
					   5.0f);
	  ImGui::DragFloat("Rotation Speed", &m_Camera2.RotationSpeed, 0.1f, 0.1f,
					   5.0f);
	  ImGui::Checkbox("Flip y", &m_Camera2.flipY);
	  ImGui::DragFloat("Fov", &fov, 0.1f, 0.1f, 179.0f);
	  ImGui::DragFloat3("Position", pos, 0.1f, -1000.0f, 1000.0f);
	  ImGui::DragFloat3("Rotation", rot, 0.1f, -1000.0f, 1000.0f);

	  m_Camera2.SetPosition(glm::vec3 {pos[0], pos[1], pos[2]});
	  m_Camera2.SetRotation(glm::vec3(rot[0], rot[1], rot[2]));
	  m_Camera2.SetFov(fov);
	}

	ImGui::End();
  }
  void EditorLayer::OnUpdate(float ts) {
	UpdateCamera(ts);

	Renderer3D::BeginDraw();
	Renderer3D::BeginScene(m_Camera2);
	for (auto& meshData : m_EditorScene->MeshDatas) {
	  Renderer3D::DrawModel(meshData.ModelPtr.get(), meshData.Transform());
	}
	Renderer3D::EndScene();
	Renderer3D::EndDraw();
  }
  void EditorLayer::OnEvent(Event& e) {
	EventDispatcher dispatcher(e);
	dispatcher.Dispatch<MouseMovedEvent>(
		BIND_EVENT_FN(EditorLayer::OnMouseMoved));
  }
  bool EditorLayer::OnMousePressed(MouseButtonPressedEvent& event) {
	return true;
  }
  bool EditorLayer::OnMouseMoved(MouseMovedEvent& event) {
	if (Input::IsMouseButtonDown(MouseButton::Right)) {
	  Input::SetCursorMode(CursorMode::Locked);
	  int32_t dx = (int32_t)m_Camera2.LastMouseState.x - event.GetX();
	  int32_t dy = (int32_t)m_Camera2.LastMouseState.y - event.GetY();
	  m_Camera2.LastMouseState.x = dx;
	  m_Camera2.LastMouseState.y = dy;
	  m_Camera2.Rotate(glm::vec3(dy * m_Camera2.RotationSpeed,
								 -dx * m_Camera2.RotationSpeed, 0.0f));
	} else {
	  Input::SetCursorMode(CursorMode::Normal);
	}
	return true;
  }
  void EditorLayer::UpdateCamera(float ts) {
	if (Input::IsKeyDown(KeyCode::W)) {
	  m_Camera2.MoveUp();
	}
	if (Input::IsKeyDown(KeyCode::S)) {
	  m_Camera2.MoveDown();
	}
	if (Input::IsKeyDown(KeyCode::D)) {
	  m_Camera2.MoveRight();
	}
	if (Input::IsKeyDown(KeyCode::A)) {
	  m_Camera2.MoveLeft();
	}
	m_Camera2.Update(ts);

	if (Input::IsMouseButtonDown(MouseButton::Right)) {
	  const glm::vec2& mouse {Input::GetMousePosition().x,
							  Input::GetMousePosition().y};
	  glm::vec2 delta = (mouse - m_Camera.m_InitialMousePosition) * 0.003f;
	  m_Camera.m_InitialMousePosition = mouse;
	  m_Camera.Rotate(delta);
	}
  }
}  // namespace FooGame
