#include "infrastructure/DatabaseRepository.h"

namespace micran_tree_cache::infrastructure {

using domain::ApplyResult;
using domain::ChangeSet;
using domain::NodeId;
using domain::NodeSnapshot;

DatabaseRepository::DatabaseRepository(InMemoryDatabase& database) : database_{database} {}

std::optional<NodeSnapshot> DatabaseRepository::loadNode(NodeId id) {
    return database_.fetchNode(id);
}

std::vector<NodeSnapshot> DatabaseRepository::loadRoots() {
    return database_.fetchRoots();
}

std::vector<NodeSnapshot> DatabaseRepository::loadChildren(NodeId parentId) {
    return database_.fetchChildren(parentId);
}

ApplyResult DatabaseRepository::applyChanges(const ChangeSet& changes) {
    return database_.applyChanges(changes);
}

} // namespace micran_tree_cache::infrastructure