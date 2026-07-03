#ifndef MICRAN_TREE_CACHE_PRESENTATION_I_TREE_DATA_PROVIDER_H
#define MICRAN_TREE_CACHE_PRESENTATION_I_TREE_DATA_PROVIDER_H

#include "domain/NodeId.h"
#include "domain/NodeStatus.h"

#include <QString>
#include <cstddef>

namespace micran_tree_cache::presentation {
class ITreeDataProvider {
public:
    using NodeHandle = void*;

    virtual ~ITreeDataProvider() = default;

    [[nodiscard]] virtual int rootCount() const = 0;
    [[nodiscard]] virtual NodeHandle rootAt(int index) const = 0;
    [[nodiscard]] virtual int childCount(NodeHandle node) const = 0;
    [[nodiscard]] virtual NodeHandle childAt(NodeHandle node, int index) const = 0;
    [[nodiscard]] virtual NodeHandle parentOf(NodeHandle node) const = 0;
    [[nodiscard]] virtual int indexOfChild(NodeHandle node) const = 0;

    [[nodiscard]] virtual QString payload(NodeHandle node) const = 0;
    [[nodiscard]] virtual domain::NodeStatus status(NodeHandle node) const = 0;
    [[nodiscard]] virtual bool hasUnloadedChildren(NodeHandle node) const = 0;
    [[nodiscard]] virtual domain::NodeId id(NodeHandle node) const = 0;

protected:
    ITreeDataProvider() = default;
    ITreeDataProvider(const ITreeDataProvider&) = default;
    ITreeDataProvider& operator=(const ITreeDataProvider&) = default;
    ITreeDataProvider(ITreeDataProvider&&) = default;
    ITreeDataProvider& operator=(ITreeDataProvider&&) = default;
};

} // namespace micran_tree_cache::presentation

#endif // MICRAN_TREE_CACHE_PRESENTATION_I_TREE_DATA_PROVIDER_H