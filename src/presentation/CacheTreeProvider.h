#ifndef MICRAN_TREE_CACHE_PRESENTATION_CACHE_TREE_PROVIDER_H
#define MICRAN_TREE_CACHE_PRESENTATION_CACHE_TREE_PROVIDER_H

#include "application/CacheService.h"
#include "presentation/ITreeDataProvider.h"

namespace micran_tree_cache::presentation {
class CacheTreeProvider final : public ITreeDataProvider {
public:
    explicit CacheTreeProvider(const application::CacheService& cache);

    [[nodiscard]] int rootCount() const override;
    [[nodiscard]] NodeHandle rootAt(int index) const override;
    [[nodiscard]] int childCount(NodeHandle node) const override;
    [[nodiscard]] NodeHandle childAt(NodeHandle node, int index) const override;
    [[nodiscard]] NodeHandle parentOf(NodeHandle node) const override;
    [[nodiscard]] int indexOfChild(NodeHandle node) const override;

    [[nodiscard]] QString payload(NodeHandle node) const override;
    [[nodiscard]] domain::NodeStatus status(NodeHandle node) const override;
    [[nodiscard]] bool hasUnloadedChildren(NodeHandle node) const override;
    [[nodiscard]] domain::NodeId id(NodeHandle node) const override;

private:
    const application::CacheService& cache_;
};

} // namespace micran_tree_cache::presentation

#endif // MICRAN_TREE_CACHE_PRESENTATION_CACHE_TREE_PROVIDER_H