#include "presentation/NodeItemDelegate.h"

#include "domain/NodeStatus.h"
#include "presentation/TreeModel.h"

#include <QApplication>
#include <QPainter>
#include <QPolygon>
#include <QStyleOptionViewItem>

namespace micran_tree_cache::presentation {

using domain::NodeStatus;

namespace {

const QColor kNewColor{0x2E, 0x7D, 0x32};
const QColor kModifiedColor{0xE6, 0x5A, 0x00};
const QColor kDeletedColor{0x9E, 0x9E, 0x9E};

constexpr int kMarkerWidth = 18;

NodeStatus statusOf(const QModelIndex& index) {
    const int raw = index.data(TreeModel::StatusRole).toInt();
    return static_cast<NodeStatus>(raw);
}

QColor colorForStatus(NodeStatus status) {
    switch (status) {
    case NodeStatus::New:
        return kNewColor;
    case NodeStatus::Modified:
        return kModifiedColor;
    case NodeStatus::Deleted:
        return kDeletedColor;
    case NodeStatus::Unchanged:
    default:
        return {};
    }
}

void paintStatusMarker(QPainter* painter, const QRect& rect, NodeStatus status,
                       const QColor& color) {
    painter->save();
    QPen pen{color};
    pen.setWidth(2);
    painter->setPen(pen);

    const QPoint center = rect.center();
    const int half = 4;

    switch (status) {
    case NodeStatus::New:
        painter->drawLine(center.x() - half, center.y(), center.x() + half, center.y());
        painter->drawLine(center.x(), center.y() - half, center.x(), center.y() + half);
        break;
    case NodeStatus::Modified:
        painter->setBrush(color);
        {
            QPolygon diamond;
            diamond << QPoint{center.x(), center.y() - half}
                    << QPoint{center.x() + half, center.y()}
                    << QPoint{center.x(), center.y() + half}
                    << QPoint{center.x() - half, center.y()};
            painter->drawPolygon(diamond);
        }
        break;
    case NodeStatus::Deleted:
        painter->drawLine(center.x() - half, center.y() - half, center.x() + half,
                          center.y() + half);
        painter->drawLine(center.x() - half, center.y() + half, center.x() + half,
                          center.y() - half);
        break;
    case NodeStatus::Unchanged:
    default:
        break;
    }
    painter->restore();
}

} // namespace

NodeItemDelegate::NodeItemDelegate(QObject* parent) : QStyledItemDelegate{parent} {}

void NodeItemDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option,
                             const QModelIndex& index) const {
    const NodeStatus status = statusOf(index);
    const bool hasUnloaded = index.data(TreeModel::HasUnloadedChildrenRole).toBool();

    QStyleOptionViewItem opt = option;
    initStyleOption(&opt, index);

    if (hasUnloaded) {
        opt.text = opt.text + QStringLiteral("  …");
    }

    const QColor statusColor = colorForStatus(status);
    if (statusColor.isValid()) {
        opt.palette.setColor(QPalette::Text, statusColor);
        opt.palette.setColor(QPalette::HighlightedText, statusColor);
    }
    if (status == NodeStatus::Deleted) {
        opt.font.setStrikeOut(true);
    }

    const bool showMarker = status != NodeStatus::Unchanged;
    if (showMarker) {
        opt.rect.adjust(kMarkerWidth, 0, 0, 0);
    }

    QStyle* style = opt.widget != nullptr ? opt.widget->style() : QApplication::style();
    style->drawControl(QStyle::CE_ItemViewItem, &opt, painter, opt.widget);

    if (showMarker) {
        const QRect markerRect{option.rect.left(), option.rect.top(), kMarkerWidth,
                               option.rect.height()};
        paintStatusMarker(painter, markerRect, status, statusColor);
    }
}

QSize NodeItemDelegate::sizeHint(const QStyleOptionViewItem& option,
                                 const QModelIndex& index) const {
    QSize base = QStyledItemDelegate::sizeHint(option, index);
    base.setHeight(base.height() + 4);
    return base;
}

} // namespace micran_tree_cache::presentation