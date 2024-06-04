#pragma once
#include "../../Defines.h"
#include <cstdint>
#include <type_traits>
namespace ENGINE_NAMESPACE
{
    template <typename IndexType, typename UniqueTag>
    struct IndexWrapper
    {
            IndexWrapper() noexcept {}

            template <typename T,
                      typename = typename std::enable_if<std::is_integral<T>::value, T>::type>
            explicit IndexWrapper(T Value) noexcept : m_Value{static_cast<IndexType>(Value)}
            {
            }

            template <typename OtherType, typename OtherTag>
            explicit IndexWrapper(const IndexWrapper<OtherType, OtherTag>& OtherIdx) noexcept
                : IndexWrapper{static_cast<uint32_t>(OtherIdx)}
            {
            }

            explicit constexpr IndexWrapper(IndexType Value) : m_Value{Value} {}

            constexpr IndexWrapper(const IndexWrapper&) = default;
            constexpr IndexWrapper(IndexWrapper&&)      = default;

            constexpr operator uint32_t() const { return m_Value; }

            template <typename T,
                      typename = typename std::enable_if<std::is_integral<T>::value, T>::type>
            IndexWrapper& operator=(const T& Value)
            {
                m_Value = static_cast<IndexType>(Value);
                VERIFY(static_cast<T>(m_Value) == Value, "Not enough bits to store value ", Value);
                return *this;
            }

            constexpr bool operator==(const IndexWrapper& Other) const
            {
                return m_Value == Other.m_Value;
            }

            struct Hasher
            {
                    size_t operator()(const IndexWrapper& Idx) const noexcept
                    {
                        return size_t{Idx.m_Value};
                    }
            };

        private:
            IndexType m_Value = 0;
    };

    using HardwareQueueIndex = IndexWrapper<uint8_t, struct _HardwareQueueIndexTag>;
    using SoftwareQueueIndex = IndexWrapper<uint8_t, struct _SoftwareQueueIndexTag>;
    using DeviceContextIndex = IndexWrapper<uint8_t, struct _DeviceContextIndexTag>;
}  // namespace ENGINE_NAMESPACE
