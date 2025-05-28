#pragma once
#include "../Base.h"
#include <cstring>
namespace FooGame
{
    class UUID
    {
        public:
            UUID();
            UUID(u64 uuid);
            UUID(const UUID&) = default;

            operator u64() const { return m_UUID; }

        private:
            u64 m_UUID;
    };

}  // namespace FooGame

namespace std
{
    template <typename T>
    struct hash;

    template <>
    struct hash<FooGame::UUID>
    {
            std::size_t operator()(const FooGame::UUID& uuid) const { return (u64)uuid; }
    };

}  // namespace std
