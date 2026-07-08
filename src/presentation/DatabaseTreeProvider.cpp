#include "presentation/DatabaseTreeProvider.h"

#include <QString>

namespace micran_tree_cache::presentation {

using domain::NodeId;
using domain::NodeSnapshot;
using domain::NodeStatus;

DatabaseTreeProvider::DatabaseTreeProvider(domain::IDatabaseRepository& repository)
    : repository_{repository} {
    rebuild();
}

void DatabaseTreeProvider::rebuild() {
    roots_.clear();

    for (const NodeSnapshot& snapshot : repository_.loadRoots()) {
        auto root = std::make_unique<Node>();
        root->id = snapshot.id;
        root->payload = QString::fromStdString(snapshot.payload);
        root->deleted = snapshot.deleted;
        root->parent = nullptr;
        buildSubtree(root.get());
        roots_.push_back(std::move(root));
    }
}

void DatabaseTreeProvider::buildSubtree(Node* node) {
    for (const NodeSnapshot& childSnapshot : repository_.loadChildren(node->id)) {
        auto child = std::make_unique<Node>();
        child->id = childSnapshot.id;
        child->payload = QString::fromStdString(childSnapshot.payload);
        child->deleted = childSnapshot.deleted;
        child->parent = node;
        buildSubtree(child.get());
        node->children.push_back(std::move(child));
    }
}

int DatabaseTreeProvider::rootCount() const {
    return static_cast<int>(roots_.size());
}

ITreeDataProvider::NodeHandle DatabaseTreeProvider::rootAt(int index) const {
    return roots_.at(static_cast<std::size_t>(index)).get();
}

int DatabaseTreeProvider::childCount(NodeHandle node) const {
    return static_cast<int>(static_cast<Node*>(node)->children.size());
}

ITreeDataProvider::NodeHandle DatabaseTreeProvider::childAt(NodeHandle node, int index) const {
    return static_cast<Node*>(node)->children.at(static_cast<std::size_t>(index)).get();
}

ITreeDataProvider::NodeHandle DatabaseTreeProvider::parentOf(NodeHandle node) const {
    return static_cast<Node*>(node)->parent;
}

int DatabaseTreeProvider::indexOfChild(NodeHandle node) const {
    Node* self = static_cast<Node*>(node);
    const std::vector<std::unique_ptr<Node>>& siblings =
        self->parent != nullptr ? self->parent->children : roots_;

    for (std::size_t i = 0; i < siblings.size(); ++i) {
        if (siblings[i].get() == self) {
            return static_cast<int>(i);
        }
    }
    return 0;
}

QString DatabaseTreeProvider::payload(NodeHandle node) const {
    return static_cast<Node*>(node)->payload;
}

NodeStatus DatabaseTreeProvider::status(NodeHandle node) const {
    return static_cast<Node*>(node)->deleted ? NodeStatus::Deleted : NodeStatus::Unchanged;
}

bool DatabaseTreeProvider::hasUnloadedChildren(NodeHandle /*node*/) const {
    return false;
}

NodeId DatabaseTreeProvider::id(NodeHandle node) const {
    return static_cast<Node*>(node)->id;
}

} // namespace micran_tree_cache::presentation