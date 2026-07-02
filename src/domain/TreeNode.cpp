#include "domain/TreeNode.h"

#include <utility>

namespace micran_tree_cache::domain {

TreeNode::TreeNode(NodeId id, std::optional<NodeId> parentId, std::string payload,
                   NodeStatus status)
    : id_{id}, parentId_{parentId}, payload_{std::move(payload)}, status_{status} {}

TreeNode* TreeNode::attachChild(std::unique_ptr<TreeNode> child) {
    child->parent_ = this;
    children_.push_back(std::move(child));
    return children_.back().get();
}

bool TreeNode::setPayload(std::string payload) {
    if (isDeleted()) {
        return false;
    }

    payload_ = std::move(payload);

    if (status_ == NodeStatus::Unchanged) {
        status_ = NodeStatus::Modified;
    }
    return true;
}

void TreeNode::markDeletedRecursively() {
    status_ = NodeStatus::Deleted;
    for (const auto& child : children_) {
        child->markDeletedRecursively();
    }
}

} // namespace micran_tree_cache::domain