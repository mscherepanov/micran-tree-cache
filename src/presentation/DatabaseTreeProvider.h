#ifndef MICRAN_TREE_CACHE_PRESENTATION_DATABASE_TREE_PROVIDER_H
#define MICRAN_TREE_CACHE_PRESENTATION_DATABASE_TREE_PROVIDER_H

#include "domain/IDatabaseRepository.h"
#include "presentation/ITreeDataProvider.h"

#include <memory>
#include <vector>

namespace micran_tree_cache::presentation {
class DatabaseTreeProvider final : public ITreeDataProvider {
public:
    explicit DatabaseTreeProvider(domain::IDatabaseRepository& repository);

    void rebuild();

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
    struct Node {
        domain::NodeId id{domain::invalidNodeId};
        QString payload;
        bool deleted{false};
        Node* parent{nullptr};
        std::vector<std::unique_ptr<Node>> children;
    };

    void buildSubtree(Node* node);

    domain::IDatabaseRepository& repository_;
    std::vector<std::unique_ptr<Node>> roots_;
};

} // namespace micran_tree_cache::presentation

#endif // MICRAN_TREE_CACHE_PRESENTATION_DATABASE_TREE_PROVIDER_H