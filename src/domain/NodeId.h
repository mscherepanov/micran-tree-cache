#ifndef MICRAN_TREE_CACHE_DOMAIN_NODE_ID_H
#define MICRAN_TREE_CACHE_DOMAIN_NODE_ID_H

#include <cstdint>
#include <functional>

namespace micran_tree_cache::domain {
enum class NodeId : std::uint64_t {};

inline constexpr NodeId invalidNodeId{0};

[[nodiscard]] constexpr std::uint64_t toRaw(NodeId id) noexcept {
    return static_cast<std::uint64_t>(id);
}

} // namespace micran_tree_cache::domain

template <> struct std::hash<micran_tree_cache::domain::NodeId> {
    [[nodiscard]] std::size_t operator()(micran_tree_cache::domain::NodeId id) const noexcept {
        return std::hash<std::uint64_t>{}(micran_tree_cache::domain::toRaw(id));
    }
};

#endif // MICRAN_TREE_CACHE_DOMAIN_NODE_ID_H