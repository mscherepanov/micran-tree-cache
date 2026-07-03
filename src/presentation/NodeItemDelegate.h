#ifndef MICRAN_TREE_CACHE_PRESENTATION_NODE_ITEM_DELEGATE_H
#define MICRAN_TREE_CACHE_PRESENTATION_NODE_ITEM_DELEGATE_H

#include <QStyledItemDelegate>

namespace micran_tree_cache::presentation {
class NodeItemDelegate final : public QStyledItemDelegate {
    Q_OBJECT

public:
    explicit NodeItemDelegate(QObject* parent = nullptr);

    void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const override;

    [[nodiscard]] QSize sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const override;
};

} // namespace micran_tree_cache::presentation

#endif // MICRAN_TREE_CACHE_PRESENTATION_NODE_ITEM_DELEGATE_H