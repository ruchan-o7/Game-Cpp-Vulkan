#pragma once
#include <cstdint>
#include <cstring>
namespace FooGame
{
    class UUID
    {
        public:
            UUID();
            UUID(uint64_t uuid);
            UUID(const UUID&) = default;

            operator uint64_t() const { return m_UUID; }

        private:
            uint64_t m_UUID;
    };

}  // namespace FooGame

namespace std
{
    template <typename T>
    struct hash;

    template <>
    struct hash<FooGame::UUID>
    {
            std::size_t operator()(const FooGame::UUID& uuid) const
            {
                return (uint64_t)uuid;
            }
    };

}  // namespace std
