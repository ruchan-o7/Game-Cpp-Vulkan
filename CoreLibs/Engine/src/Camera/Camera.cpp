#include "Camera.h"
namespace FooGame {
  void Camera::Update(float deltaTime) {
	if (type == CameraType::firstperson) {
	  glm::vec3 camFront;
	  camFront.x =
		  -cos(glm::radians(Rotation.x)) * sin(glm::radians(Rotation.y));
	  camFront.y = sin(glm::radians(Rotation.x));
	  camFront.z =
		  cos(glm::radians(Rotation.x)) * cos(glm::radians(Rotation.y));
	  Front = glm::normalize(camFront);

	  MoveSpeed_ = deltaTime * MovementSpeed;
	}
	UpdateViewMatrix();
  };
  void Camera::MoveUp() { Position += Front * MoveSpeed_; }
  void Camera::MoveDown() { Position -= Front * MoveSpeed_; }
  void Camera::MoveLeft() {
	Position -= glm::normalize(glm::cross(Front, glm::vec3(0.0f, 1.0f, 0.0f))) *
				MoveSpeed_;
  }
  void Camera::MoveRight() {
	Position += glm::normalize(glm::cross(Front, glm::vec3(0.0f, 1.0f, 0.0f))) *
				MoveSpeed_;
  }

  void Camera::setPerspective(float fov, float aspect, float znear,
							  float zfar) {
	Fov    = fov;
	Aspect = aspect;
	ZNear  = znear;
	ZFar   = zfar;
	CalculatePerspective();
  };
  void Camera::SetFov(float fov) {
	Fov = fov;
	CalculatePerspective();
  }
  void Camera::CalculatePerspective() {
	matrices.Perspective =
		glm::perspective(glm::radians(Fov), Aspect, ZNear, ZFar);
	if (flipY) {
	  matrices.Perspective[1][1] *= -1.0f;
	}
  }

  void Camera::updateAspectRatio(float aspect) {
	Aspect = aspect;
	CalculatePerspective();
  }

  void Camera::UpdateViewMatrix() {
	glm::mat4 currentMatrix = matrices.View;

	glm::mat4 rotM = glm::mat4(1.0f);
	glm::mat4 transM;

	rotM = glm::rotate(rotM, glm::radians(Rotation.x * (flipY ? -1.0f : 1.0f)),
					   glm::vec3(1.0f, 0.0f, 0.0f));
	rotM = glm::rotate(rotM, glm::radians(Rotation.y),
					   glm::vec3(0.0f, 1.0f, 0.0f));
	rotM = glm::rotate(rotM, glm::radians(Rotation.z),
					   glm::vec3(0.0f, 0.0f, 1.0f));

	glm::vec3 translation = Position;
	if (flipY) {
	  translation.y *= -1.0f;
	}
	transM = glm::translate(glm::mat4(1.0f), translation);

	if (type == CameraType::firstperson) {
	  matrices.View = rotM * transM;
	} else {
	  matrices.View = transM * rotM;
	}

	ViewPos = glm::vec4(Position, 0.0f) * glm::vec4(-1.0f, 1.0f, -1.0f, 1.0f);
  };
}  // namespace FooGame
