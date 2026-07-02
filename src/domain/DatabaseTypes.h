#ifndef MICRAN_TREE_CACHE_DOMAIN_DATABASE_TYPES_H
#define MICRAN_TREE_CACHE_DOMAIN_DATABASE_TYPES_H

#include "domain/NodeId.h"

#include <optional>
#include <string>
#include <vector>

namespace micran_tree_cache::domain {
struct NodeSnapshot {
    NodeId id{invalidNodeId};
    std::optional<NodeId> paraentId;
    std::string payload;
    bool deleted{false};
    bool hasChildren{false};
};

struct NewNodeChange {
    NodeId id{invalidNodeId};
    std::optional<NodeId> parentId;
    std::string payload;
};

struct ModifiedNodeChange {
    NodeId id{invalidNodeId};
    std::string payload;
};

struct ChangeSet {
    std::vector<NewNodeChange> created;
    std::vector<ModifiedNodeChange> modified;
    std::vector<NodeId> deleted;
};

} // namespace micran_tree_cache::domain

#endif // MICRAN_TREE_CACHE_DOMAIN_DATABASE_TYPES_H