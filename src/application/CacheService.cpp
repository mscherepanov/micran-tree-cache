#include "application/CacheService.h"

#include <algorithm>
#include <utility>

namespace micran_tree_cache::application {

using domain::NodeId;
using domain::NodeSnapshot;
using domain::NodeStatus;
using domain::TreeNode;

CacheService::CacheService(domain::IDatabaseRepository& repository) : repository_{repository} {}

TreeNode* CacheService::findNode(NodeId id) const {
    const auto it = index_.find(id);
    return it == index_.end() ? nullptr : it->second;
}

CacheService::OperationResult CacheService::loadFromDatabase(NodeId id) {
    if (TreeNode* existing = findNode(id); existing != nullptr) {
        return OperationResult::AlreadyInCache;
    }

    const std::optional<NodeSnapshot> snapshot = repository_.loadNode(id);
    if (!snapshot.has_value()) {
        return OperationResult::NodeNotFound;
    }

    insertLoadedNode(*snapshot);
    return OperationResult::Success;
}

TreeNode* CacheService::insertLoadedNode(const NodeSnapshot& snapshot) {
    auto node =
        std::make_unique<TreeNode>(snapshot.id, snapshot.parentId, snapshot.payload,
                                   snapshot.deleted ? NodeStatus::Deleted : NodeStatus::Unchanged);
    TreeNode* raw = node.get();

    TreeNode* parent = snapshot.parentId.has_value() ? findNode(*snapshot.parentId) : nullptr;
    if (parent != nullptr) {
        parent->attachChild(std::move(node));
    } else {
        roots_.push_back(std::move(node));
    }

    index_.emplace(raw->id(), raw);

    adoptOrphans(raw);

    raw->setHasUnloadedChildren(snapshot.hasChildren && raw->children().empty());

    if (parent != nullptr) {
        refreshUnloadedFlag(parent);
    }
    return raw;
}

void CacheService::adoptOrphans(TreeNode* node) {
    std::vector<NodeId> toAdopt;
    for (const auto& root : roots_) {
        if (root->parentId() == node->id()) {
            toAdopt.push_back(root->id());
        }
    }

    for (const NodeId childId : toAdopt) {
        std::unique_ptr<TreeNode> child = detachRoot(childId);
        if (child != nullptr) {
            node->attachChild(std::move(child));
        }
    }

    if (!toAdopt.empty()) {
        refreshUnloadedFlag(node);
    }
}

void CacheService::refreshUnloadedFlag(TreeNode* node) {
    if (node == nullptr) {
        return;
    }

    if (!node->children().empty()) {
        node->setHasUnloadedChildren(false);
    }
}

CacheService::OperationResult CacheService::addChild(NodeId parentId, std::string payload) {
    TreeNode* parent = findNode(parentId);
    if (parent == nullptr) {
        return OperationResult::NodeNotFound;
    }

    if (parent->isDeleted()) {
        return OperationResult::NodeDeleted;
    }

    const NodeId newId = nextTemporaryId();
    auto child = std::make_unique<TreeNode>(newId, parentId, std::move(payload), NodeStatus::New);
    TreeNode* raw = parent->attachChild(std::move(child));
    index_.emplace(newId, raw);

    refreshUnloadedFlag(parent);
    return OperationResult::Success;
}

CacheService::OperationResult CacheService::editPayload(NodeId id, std::string payload) {
    TreeNode* node = findNode(id);
    if (node == nullptr) {
        return OperationResult::NodeNotFound;
    }

    if (!node->setPayload(std::move(payload))) {
        return OperationResult::NodeDeleted;
    }
    return OperationResult::Success;
}

CacheService::OperationResult CacheService::remove(NodeId id) {
    TreeNode* node = findNode(id);
    if (node == nullptr) {
        return OperationResult::NodeNotFound;
    }
    node->markDeletedRecursively();
    return OperationResult::Success;
}

void CacheService::clear() {
    roots_.clear();
    index_.clear();
    nextTemporaryId_ = temporaryIdBase_;
}

std::unique_ptr<TreeNode> CacheService::detachRoot(NodeId id) {
    const auto it =
        std::find_if(roots_.begin(), roots_.end(),
                     [id](const std::unique_ptr<TreeNode>& root) { return root->id() == id; });
    if (it == roots_.end()) {
        return nullptr;
    }
    std::unique_ptr<TreeNode> detached = std::move(*it);
    roots_.erase(it);
    return detached;
}

NodeId CacheService::nextTemporaryId() noexcept {
    return NodeId{nextTemporaryId_++};
}

bool CacheService::reassignId(NodeId temporaryId, NodeId persistentId) {
    const auto it = index_.find(temporaryId);
    if (it == index_.end()) {
        return false;
    }

    TreeNode* node = it->second;
    node->setId(persistentId);

    index_.erase(it);
    index_.emplace(persistentId, node);
    return true;
}

} // namespace micran_tree_cache::application