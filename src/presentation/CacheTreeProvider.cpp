#include "presentation/CacheTreeProvider.h"

#include <QString>

namespace micran_tree_cache::presentation {

using domain::NodeId;
using domain::NodeStatus;
using domain::TreeNode;

namespace {

TreeNode* asNode(ITreeDataProvider::NodeHandle handle) {
    return static_cast<TreeNode*>(handle);
}

} // namespace

CacheTreeProvider::CacheTreeProvider(const application::CacheService& cache) : cache_{cache} {}

int CacheTreeProvider::rootCount() const {
    return static_cast<int>(cache_.roots().size());
}

ITreeDataProvider::NodeHandle CacheTreeProvider::rootAt(int index) const {
    return cache_.roots().at(static_cast<std::size_t>(index)).get();
}

int CacheTreeProvider::childCount(NodeHandle node) const {
    return static_cast<int>(asNode(node)->children().size());
}

ITreeDataProvider::NodeHandle CacheTreeProvider::childAt(NodeHandle node, int index) const {
    return asNode(node)->children().at(static_cast<std::size_t>(index)).get();
}

ITreeDataProvider::NodeHandle CacheTreeProvider::parentOf(NodeHandle node) const {
    return asNode(node)->parent();
}

int CacheTreeProvider::indexOfChild(NodeHandle node) const {
    TreeNode* self = asNode(node);
    TreeNode* parent = self->parent();

    if (parent == nullptr) {
        const auto& roots = cache_.roots();
        for (std::size_t i = 0; i < roots.size(); ++i) {
            if (roots[i].get() == self) {
                return static_cast<int>(i);
            }
        }
        return 0;
    }

    const auto& siblings = parent->children();
    for (std::size_t i = 0; i < siblings.size(); ++i) {
        if (siblings[i].get() == self) {
            return static_cast<int>(i);
        }
    }
    return 0;
}

QString CacheTreeProvider::payload(NodeHandle node) const {
    return QString::fromStdString(asNode(node)->payload());
}

NodeStatus CacheTreeProvider::status(NodeHandle node) const {
    return asNode(node)->status();
}

bool CacheTreeProvider::hasUnloadedChildren(NodeHandle node) const {
    return asNode(node)->hasUnloadedChildren();
}

NodeId CacheTreeProvider::id(NodeHandle node) const {
    return asNode(node)->id();
}

} // namespace micran_tree_cache::presentation