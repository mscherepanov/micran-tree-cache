#ifndef MICRAN_TREE_CACHE_DOMAIN_TREE_NODE_H
#define MICRAN_TREE_CACHE_DOMAIN_TREE_NODE_H

#include "domain/NodeId.h"
#include "domain/NodeStatus.h"

#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace micran_tree_cache::domain {
class TreeNode {
public:
    TreeNode(NodeId id, std::optional<NodeId> parentId, std::string payload, NodeStatus status);

    TreeNode(const TreeNode&) = delete;
    TreeNode& operator=(const TreeNode&) = delete;
    TreeNode(TreeNode&&) noexcept = default;
    TreeNode& operator=(TreeNode&&) noexcept = default;
    ~TreeNode() = default;

    [[nodiscard]] NodeId id() const noexcept { return id_; }
    [[nodiscard]] std::optional<NodeId> parentId() const noexcept { return parentId_; }
    [[nodiscard]] const std::string& payload() const noexcept { return payload_; }
    [[nodiscard]] NodeStatus status() const noexcept { return status_; }
    [[nodiscard]] TreeNode* parent() const noexcept { return parent_; }
    [[nodiscard]] bool hasUnloadedChildren() const noexcept { return hasUnloadedChildren_; }
    [[nodiscard]] const std::vector<std::unique_ptr<TreeNode>>& children() const noexcept {
        return children_;
    }
    [[nodiscard]] bool isDeleted() const noexcept { return status_ == NodeStatus::Deleted; }
    [[nodiscard]] bool isEditable() const noexcept { return !isDeleted(); }

    TreeNode* attachChild(std::unique_ptr<TreeNode> child);

    bool setPayload(std::string payload);

    void markDeletedRecursively();
    void setHasUnloadedChildren(bool value) noexcept { hasUnloadedChildren_ = value; }
    void setStatus(NodeStatus status) noexcept { status_ = status; }

private:
    NodeId id_;
    std::optional<NodeId> parentId_;
    std::string payload_;
    NodeStatus status_;
    bool hasUnloadedChildren_{false};

    TreeNode* parent_{nullptr};
    std::vector<std::unique_ptr<TreeNode>> children_;
};

} // namespace micran_tree_cache::domain

#endif // MICRAN_TREE_CACHE_DOMAIN_TREE_NODE_H