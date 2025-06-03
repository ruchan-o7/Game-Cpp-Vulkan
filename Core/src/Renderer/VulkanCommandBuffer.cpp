#include "VulkanCommandBuffer.h"
#include "../Core/Assert.h"

namespace fg {

VulkanCommandBuffer::VulkanCommandBuffer() noexcept {
  m_Barriers.reserve(32);
}

void VulkanCommandBuffer::FlushBarriers() {
  FOO_ASSERT(false);
}
}  // namespace fg
