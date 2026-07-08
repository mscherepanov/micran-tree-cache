#ifndef MICRAN_TREE_CACHE_DOMAIN_NODE_STATUS_H
#define MICRAN_TREE_CACHE_DOMAIN_NODE_STATUS_H

#include <cstdint>

namespace micran_tree_cache::domain {
enum class NodeStatus : std::uint8_t { Unchanged, New, Modified, Deleted };

} // namespace micran_tree_cache::domain

#endif // MICRAN_TREE_CACHE_DOMAIN_NODE_STATUS_H