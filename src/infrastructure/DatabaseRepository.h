#ifndef MICRAN_TREE_CACHE_INFRASTRUCTURE_DATABASE_REPOSITORY_H
#define MICRAN_TREE_CACHE_INFRASTRUCTURE_DATABASE_REPOSITORY_H

#include "domain/IDatabaseRepository.h"
#include "infrastructure/InMemoryDatabase.h"

namespace micran_tree_cache::infrastructure {
class DatabaseRepository final : public domain::IDatabaseRepository {
public:
    explicit DatabaseRepository(InMemoryDatabase& database);

    [[nodiscard]] std::optional<domain::NodeSnapshot> loadNode(domain::NodeId id) override;
    [[nodiscard]] std::vector<domain::NodeSnapshot> loadRoots() override;
    [[nodiscard]] std::vector<domain::NodeSnapshot> loadChildren(domain::NodeId parentId) override;
    domain::ApplyResult applyChanges(const domain::ChangeSet& changes) override;

private:
    InMemoryDatabase& database_;
};

} // namespace micran_tree_cache::infrastructure

#endif // MICRAN_TREE_CACHE_INFRASTRUCTURE_DATABASE_REPOSITORY_H