#pragma once
#include "src/Core/Base.h"
#include "src/Core/UUID.h"

#include "src/File/FileHelper.h"

#include "src/Scene/Component.h"
#include "src/Scene/Entity.h"
#include "src/Scene/Scene.h"
#include "src/Scene/ScriptableEntity.h"

#include "src/Engine/Camera/Camera.h"
#include "src/Engine/Camera/PerspectiveCamera.h"
#include "src/Engine/Camera/OrthographicCamera.h"

#include "src/Engine/Core/Types.h"
#include "src/Engine/Core/EngineFactory.h"
#include "src/Engine/Core/RenderDevice.h"
#include "src/Engine/Core/TypeConversion.h"
#include "src/Engine/Core/VulkanBuffer.h"
#include "src/Engine/Core/VulkanCommandBuffer.h"
#include "src/Engine/Core/VulkanCommandPoolManager.h"
#include "src/Engine/Core/VulkanDeviceContext.h"
#include "src/Engine/Core/VulkanInstance.h"
#include "src/Engine/Core/VulkanLogicalDevice.h"
#include "src/Engine/Core/VulkanPhysicalDevice.h"
#include "src/Engine/Core/VulkanPipeline.h"
#include "src/Engine/Core/VulkanRenderpass.h"
#include "src/Engine/Core/VulkanSwapchain.h"
#include "src/Engine/Core/VulkanTexture.h"

#include "src/Engine/Engine/Backend.h"
#include "src/Engine/Engine/Renderer2D.h"
#include "src/Engine/Engine/Renderer3D.h"
#include "src/Engine/Engine/Shader.h"

#include "src/Engine/Events/ApplicationEvent.h"
#include "src/Engine/Events/Event.h"
#include "src/Engine/Events/KeyEvent.h"
#include "src/Engine/Events/MouseMovedEvent.h"

#include "src/Engine/Geometry/AssetLoader.h"
#include "src/Engine/Geometry/Material.h"
#include "src/Engine/Geometry/Mesh.h"
#include "src/Engine/Geometry/Model.h"
#include "src/Engine/Geometry/QuadVertex.h"
#include "src/Engine/Geometry/Vertex.h"

#include "src/Engine/Input/Input.h"
#include "src/Engine/Input/KeyCodes.h"

#include "src/Engine/Window/Window.h"
