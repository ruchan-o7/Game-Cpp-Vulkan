//
// Created by jcead on 20.05.2024.
//
#pragma once
#include <deque>
#include <functional>
#include <vulkan/vulkan.h>
namespace FooGame
{

    class DeletionQueue
    {
        public:
            using VoidCallback = std::function<void(VkDevice)>;
            std::deque<VoidCallback> deletors;
            void PushFunction(VoidCallback&& function)
            {
                deletors.push_back(function);
            }
            void Flush(VkDevice device)
            {
                for (auto func : deletors)
                {
                    func(device);
                }
                deletors.clear();
            }
    };
    class DeletionVector
    {
        public:
            using Callback = std::function<void(VkDevice)>;
            std::vector<Callback> deletors;
            void PushFunction(Callback&& function)
            {
                deletors.push_back(function);
            }
            void Flush(VkDevice device)
            {
                for (int i = deletors.size() - 1; i >= 0; i--)
                {
                    deletors[i](device);
                }
                deletors.clear();
            }
    };
}  // namespace FooGame
