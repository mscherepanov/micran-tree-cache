#ifndef MICRAN_TREE_CACHE_INFRASTRUCTURE_IN_MEMORY_DATABASE_H
#define MICRAN_TREE_CACHE_INFRASTRUCTURE_IN_MEMORY_DATABASE_H

#include "domain/DatabaseTypes.h"
#include "domain/IDatabaseResetter.h"
#include "domain/NodeId.h"

#include <cstdint>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

namespace micran_tree_cache::infrastructure {
class InMemoryDatabase : public domain::IDatabaseResetter {
public:
    InMemoryDatabase();

    [[nodiscard]] std::optional<domain::NodeSnapshot> fetchNode(domain::NodeId id) const;
    [[nodiscard]] std::vector<domain::NodeSnapshot> fetchRoots() const;
    [[nodiscard]] std::vector<domain::NodeSnapshot> fetchChildren(domain::NodeId parentId) const;

    domain::ApplyResult applyChanges(const domain::ChangeSet& changes);

    void reset();
    void resetToInitialState() override { reset(); }

private:
    struct Record {
        std::optional<domain::NodeId> parentId;
        std::string payload;
        bool deleted{false};
    };

    [[nodiscard]] domain::NodeSnapshot makeSnapshot(domain::NodeId id, const Record& record) const;
    [[nodiscard]] bool hasChildren(domain::NodeId id) const;

    void markDeletedRecursively(domain::NodeId id);
    void seedTestData();

    domain::NodeId insertRecord(std::optional<domain::NodeId> parentId, std::string payload);

    std::unordered_map<domain::NodeId, Record> records_;
    std::uint64_t nextId_{1};
};

} // namespace micran_tree_cache::infrastructure

#endif // MICRAN_TREE_CACHE_INFRASTRUCTURE_IN_MEMORY_DATABASE_H