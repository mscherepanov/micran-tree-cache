#include "infrastructure/InMemoryDatabase.h"

#include <algorithm>
#include <utility>

namespace micran_tree_cache::infrastructure {

using domain::ApplyResult;
using domain::ChangeSet;
using domain::IdAssigment;
using domain::NodeId;
using domain::NodeSnapshot;

InMemoryDatabase::InMemoryDatabase() {
    seedTestData();
}

std::optional<NodeSnapshot> InMemoryDatabase::fetchNode(NodeId id) const {
    const auto it = records_.find(id);
    if (it == records_.end()) {
        return std::nullopt;
    }
    return makeSnapshot(id, it->second);
}

std::vector<NodeSnapshot> InMemoryDatabase::fetchRoots() const {
    std::vector<NodeSnapshot> roots;
    for (const auto& [id, record] : records_) {
        if (!record.parentId.has_value()) {
            roots.push_back(makeSnapshot(id, record));
        }
    }
    return roots;
}

std::vector<NodeSnapshot> InMemoryDatabase::fetchChildren(NodeId parentId) const {
    std::vector<NodeSnapshot> children;
    for (const auto& [id, record] : records_) {
        if (record.parentId == parentId) {
            children.push_back(makeSnapshot(id, record));
        }
    }
    return children;
}

ApplyResult InMemoryDatabase::applyChanges(const ChangeSet& changes) {
    ApplyResult result;

    std::unordered_map<NodeId, NodeId> temporaryToPersistent;

    for (const auto& created : changes.created) {
        std::optional<NodeId> parentId = created.parentId;
        if (parentId.has_value()) {
            const auto mapped = temporaryToPersistent.find(*parentId);
            if (mapped != temporaryToPersistent.end()) {
                parentId = mapped->second;
            }
        }

        const NodeId persistentId = insertRecord(parentId, created.payload);
        temporaryToPersistent.emplace(created.id, persistentId);
        result.assignedIds.push_back(IdAssigment{created.id, persistentId});
    }

    for (const auto& modified : changes.modified) {
        const auto it = records_.find(modified.id);
        if (it != records_.end() && !it->second.deleted) {
            it->second.payload = modified.payload;
        }
    }

    for (const NodeId id : changes.deleted) {
        markDeletedRecursively(id);
    }

    return result;
}

void InMemoryDatabase::reset() {
    records_.clear();
    nextId_ = 1;
    seedTestData();
}

NodeSnapshot InMemoryDatabase::makeSnapshot(NodeId id, const Record& record) const {
    NodeSnapshot snapshot;
    snapshot.id = id;
    snapshot.parentId = record.parentId;
    snapshot.payload = record.payload;
    snapshot.deleted = record.deleted;
    snapshot.hasChildren = hasChildren(id);
    return snapshot;
}

bool InMemoryDatabase::hasChildren(NodeId id) const {
    return std::ranges::any_of(records_,
                               [id](const auto& entry) { return entry.second.parentId == id; });
}

void InMemoryDatabase::markDeletedRecursively(NodeId id) {
    const auto it = records_.find(id);
    if (it == records_.end()) {
        return;
    }
    it->second.deleted = true;

    std::vector<NodeId> childIds;
    for (const auto& [childId, record] : records_) {
        if (record.parentId == id) {
            childIds.push_back(childId);
        }
    }
    for (const NodeId childId : childIds) {
        markDeletedRecursively(childId);
    }
}

NodeId InMemoryDatabase::insertRecord(std::optional<NodeId> parentId, std::string payload) {
    const NodeId id{nextId_++};
    records_.emplace(id, Record{parentId, std::move(payload), false});
    return id;
}

void InMemoryDatabase::seedTestData() {
    const NodeId vehicles = insertRecord(std::nullopt, "Автомобили");

    const NodeId sedan = insertRecord(vehicles, "Седаны");
    const NodeId suv = insertRecord(vehicles, "Кроссоверы");

    const NodeId bmw = insertRecord(sedan, "BMW");
    const NodeId toyota = insertRecord(sedan, "Toyota");
    const NodeId audi = insertRecord(suv, "Audi");

    const NodeId m3 = insertRecord(bmw, "M3");
    const NodeId series5 = insertRecord(bmw, "5 Series");
    const NodeId camry = insertRecord(toyota, "Camry");
    const NodeId q7 = insertRecord(audi, "Q7");

    insertRecord(m3, "Competition");

    static_cast<void>(series5);
    static_cast<void>(camry);
    static_cast<void>(q7);
}

} // namespace micran_tree_cache::infrastructure