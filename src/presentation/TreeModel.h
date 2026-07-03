#ifndef MICRAN_TREE_CACHE_PRESENTATION_TREE_MODEL_H
#define MICRAN_TREE_CACHE_PRESENTATION_TREE_MODEL_H

#include "domain/NodeId.h"
#include "presentation/ITreeDataProvider.h"

#include <QAbstractItemModel>
#include <QModelIndex>
#include <QVariant>
#include <memory>

namespace micran_tree_cache::presentation {
class TreeModel final : public QAbstractItemModel {
    Q_OBJECT

public:
    enum Roles {
        StatusRole = Qt::UserRole + 1,
        HasUnloadedChildrenRole,
        NodeIdRole
    };

    explicit TreeModel(std::unique_ptr<ITreeDataProvider> provider, QObject* parent = nullptr);
    ~TreeModel() override;

    [[nodiscard]] QModelIndex index(int row, int column, const QModelIndex& parent) const override;
    [[nodiscard]] QModelIndex parent(const QModelIndex& child) const override;
    [[nodiscard]] int rowCount(const QModelIndex& parent) const override;
    [[nodiscard]] int columnCount(const QModelIndex& parent) const override;
    [[nodiscard]] QVariant data(const QModelIndex& index, int role) const override;
    [[nodiscard]] QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

    void refreshAll();
    void notifyNodeChanged(domain::NodeId id);

    [[nodiscard]] const ITreeDataProvider& provider() const { return *provider_; }
    [[nodiscard]] QModelIndex indexForId(domain::NodeId id) const;

private:
    [[nodiscard]] ITreeDataProvider::NodeHandle handleForIndex(const QModelIndex& index) const;
    [[nodiscard]] QModelIndex searchIndex(domain::NodeId id, const QModelIndex& parent) const;

    std::unique_ptr<ITreeDataProvider> provider_;
};

} // namespace micran_tree_cache::presentation

#endif // MICRAN_TREE_CACHE_PRESENTATION_TREE_MODEL_H