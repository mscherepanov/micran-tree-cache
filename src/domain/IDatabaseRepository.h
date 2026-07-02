#ifndef MICRAN_TREE_CACHE_DOMAIN_I_DATABASE_REPOSITORY_H
#define MICRAN_TREE_CACHE_DOMAIN_I_DATABASE_REPOSITORY_H

#include "domain/DatabaseTypes.h"
#include "domain/NodeId.h"

#include <optional>
#include <vector>

namespace micran_tree_cache::domain {
class IDatabaseRepository {
public:
    virtual ~IDatabaseRepository() = default;

    [[nodiscard]] virtual std::optional<NodeSnapshot> loadNode(NodeId id) = 0;
    [[nodiscard]] virtual std::vector<NodeSnapshot> loadRoots() = 0;
    [[nodiscard]] virtual std::vector<NodeSnapshot> loadChildren(NodeId parentId) = 0;

    virtual void applyChanges(const ChangeSet& changes) = 0;

protected:
    IDatabaseRepository() = default;
    IDatabaseRepository(const IDatabaseRepository&) = default;
    IDatabaseRepository& operator=(const IDatabaseRepository&) = default;
    IDatabaseRepository(IDatabaseRepository&&) = default;
    IDatabaseRepository& operator=(IDatabaseRepository&&) = default;
};

} // namespace micran_tree_cache::domain

#endif // MICRAN_TREE_CACHE_DOMAIN_I_DATABASE_REPOSITORY_H