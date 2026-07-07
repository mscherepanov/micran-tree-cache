#include "application/SyncService.h"

#include "application/CacheService.h"
#include "domain/IDatabaseRepository.h"
#include "domain/TreeNode.h"

namespace micran_tree_cache::application {

using domain::ApplyResult;
using domain::ChangeSet;
using domain::ModifiedNodeChange;
using domain::NewNodeChange;
using domain::NodeStatus;
using domain::TreeNode;

SyncService::SyncService(CacheService& cache, domain::IDatabaseRepository& repository)
    : cache_{cache}, repository_{repository} {}

SyncService::Result SyncService::apply() {
    ChangeSet changes;
    Result summary;

    for (const auto& root : cache_.roots()) {
        collectChanges(root.get(), changes, summary);
    }

    const ApplyResult applyResult = repository_.applyChanges(changes);

    reconcileCache(applyResult);

    return summary;
}

void SyncService::collectChanges(const TreeNode* node, ChangeSet& changes, Result& summary) const {
    switch (node->status()) {
    case NodeStatus::New:
        changes.created.push_back(NewNodeChange{node->id(), node->parentId(), node->payload()});
        ++summary.created;
        break;
    case NodeStatus::Modified:
        changes.modified.push_back(ModifiedNodeChange{node->id(), node->payload()});
        ++summary.modified;
        break;
    case NodeStatus::Deleted:
        changes.deleted.push_back(node->id());
        ++summary.deleted;
        break;
    case NodeStatus::Unchanged:
        break;
    }

    for (const auto& child : node->children()) {
        collectChanges(child.get(), changes, summary);
    }
}

void SyncService::reconcileCache(const ApplyResult& applyResult) {
    for (const auto& assignment : applyResult.assignedIds) {
        cache_.reassignId(assignment.temporaryId, assignment.persistentId);
    }

    for (const auto& root : cache_.roots()) {
        markSubtreeSynced(root.get());
    }
}

void SyncService::markSubtreeSynced(TreeNode* node) {
    if (node->status() == NodeStatus::New || node->status() == NodeStatus::Modified) {
        node->setStatus(NodeStatus::Unchanged);
    }
    for (const auto& child : node->children()) {
        markSubtreeSynced(child.get());
    }
}

} // namespace micran_tree_cache::application