#pragma once
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>
namespace FooGame {
  class Camera {
	public:
	  enum CameraType {
		lookat,
		firstperson
	  };
	  CameraType type = CameraType::firstperson;

	  glm::vec3 Rotation       = glm::vec3();
	  glm::vec3 Position       = glm::vec3();
	  glm::vec4 ViewPos        = glm::vec4();
	  glm::vec2 LastMouseState = glm::vec2();
	  glm::vec3 Front          = glm::vec3();

	  float RotationSpeed = 0.01f;
	  float MovementSpeed = 0.01f;

	  float Fov, Aspect;
	  float ZNear, ZFar;
	  bool  flipY = false;
	  struct {
		  glm::mat4 Perspective;
		  glm::mat4 View;
	  } matrices;
	  void MoveUp();
	  void MoveDown();
	  void MoveLeft();
	  void MoveRight();

	  void UpdateViewMatrix();

	  const float GetNearClip() const { return ZNear; }

	  const float GetFarClip() const { return ZFar; }

	  void setPerspective(float fov, float aspect, float znear, float zfar);

	  void updateAspectRatio(float aspect);

	  void SetPosition(glm::vec3 position) {
		this->Position = position;
		UpdateViewMatrix();
	  }
	  void SetFov(float fov);
	  void SetRotation(glm::vec3 rotation) {
		this->Rotation = rotation;
		UpdateViewMatrix();
	  }

	  void Rotate(glm::vec3 delta) {
		this->Rotation += delta;
		UpdateViewMatrix();
	  }

	  void SetTranslatin(glm::vec3 translation) {
		this->Position = translation;
		UpdateViewMatrix();
	  };

	  void Translate(glm::vec3 delta) {
		this->Position += delta;
		UpdateViewMatrix();
	  }

	  void SetRotationSpeed(float rotationSpeed) {
		this->RotationSpeed = rotationSpeed;
	  }

	  void SetMovementSpeed(float movementSpeed) {
		this->MovementSpeed = movementSpeed;
	  }

	  void Update(float deltaTime);

	private:
	  void  CalculatePerspective();
	  float MoveSpeed_;
  };
}  // namespace FooGame
