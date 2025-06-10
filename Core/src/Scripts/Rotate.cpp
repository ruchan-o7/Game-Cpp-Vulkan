#include "Rotate.h"
#include <cmath>
#include "../Scene/Component.h"
#include "../Core/Time.h"
namespace FooGame::Script {

void RotateScript::OnCreate() {
  m_Transform = &GetComponent<TransformComponent>();
}
void RotateScript::OnUpdate(float ts) {
  double time = Time::CurrentTime();
  m_Transform->Translation.x = sin(time);
}

}  // namespace FooGame::Script
