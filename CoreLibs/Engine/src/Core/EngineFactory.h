#pragma once
#include "../Defines.h"
#include "Types.h"
#include "VulkanPhysicalDevice.h"

namespace ENGINE_NAMESPACE
{
    class EngineFactory
    {
        public:
            static EngineFactory* GetInstance()
            {
                static EngineFactory fac;
                return &fac;
            }
            void Init(const EngineCreateInfo& ci);

        private:
            EngineCreateInfo m_Ci;
    };
    GraphicsAdapterInfo GetPhysicalDeviceGraphicsAdapterInfo(const VulkanPhysicalDevice& pDevice);
}  // namespace ENGINE_NAMESPACE
