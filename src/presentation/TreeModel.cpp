#include "presentation/TreeModel.h"

#include <QObject>
#include <utility>

namespace micran_tree_cache::presentation {

using domain::NodeId;

TreeModel::TreeModel(std::unique_ptr<ITreeDataProvider> provider, QObject* parent)
    : QAbstractItemModel{parent}, provider_{std::move(provider)} {}

TreeModel::~TreeModel() = default;

ITreeDataProvider::NodeHandle TreeModel::handleForIndex(const QModelIndex& index) const {
    return index.isValid() ? index.internalPointer() : nullptr;
}

QModelIndex TreeModel::index(int row, int column, const QModelIndex& parent) const {
    if (!hasIndex(row, column, parent)) {
        return {};
    }

    ITreeDataProvider::NodeHandle parentHandle = handleForIndex(parent);
    ITreeDataProvider::NodeHandle childHandle =
        parentHandle == nullptr ? provider_->rootAt(row) : provider_->childAt(parentHandle, row);

    return createIndex(row, column, childHandle);
}

QModelIndex TreeModel::parent(const QModelIndex& child) const {
    ITreeDataProvider::NodeHandle childHandle = handleForIndex(child);
    if (childHandle == nullptr) {
        return {};
    }

    ITreeDataProvider::NodeHandle parentHandle = provider_->parentOf(childHandle);
    if (parentHandle == nullptr) {
        return {};
    }

    const int row = provider_->indexOfChild(parentHandle);
    return createIndex(row, 0, parentHandle);
}

int TreeModel::rowCount(const QModelIndex& parent) const {
    if (parent.column() > 0) {
        return 0;
    }
    ITreeDataProvider::NodeHandle parentHandle = handleForIndex(parent);
    return parentHandle == nullptr ? provider_->rootCount() : provider_->childCount(parentHandle);
}

int TreeModel::columnCount(const QModelIndex& /*parent*/) const {
    return 1;
}

QVariant TreeModel::data(const QModelIndex& index, int role) const {
    ITreeDataProvider::NodeHandle handle = handleForIndex(index);
    if (handle == nullptr) {
        return {};
    }

    switch (role) {
    case Qt::DisplayRole:
        return provider_->payload(handle);
    case StatusRole:
        return static_cast<int>(provider_->status(handle));
    case HasUnloadedChildrenRole:
        return provider_->hasUnloadedChildren(handle);
    case NodeIdRole:
        return static_cast<qulonglong>(domain::toRaw(provider_->id(handle)));
    default:
        return {};
    }
}

QVariant TreeModel::headerData(int section, Qt::Orientation orientation, int role) const {
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole && section == 0) {
        return QObject::tr("Элемент");
    }
    return {};
}

Qt::ItemFlags TreeModel::flags(const QModelIndex& index) const {
    Qt::ItemFlags base = QAbstractItemModel::flags(index);
    if (!index.isValid()) {
        return base;
    }

    if (editCallback_) {
        const auto status = static_cast<domain::NodeStatus>(index.data(StatusRole).toInt());
        if (status != domain::NodeStatus::Deleted) {
            base |= Qt::ItemIsEditable;
        }
    }
    return base;
}

bool TreeModel::setData(const QModelIndex& index, const QVariant& value, int role) {
    if (!index.isValid() || role != Qt::EditRole || !editCallback_) {
        return false;
    }

    ITreeDataProvider::NodeHandle handle = handleForIndex(index);
    if (handle == nullptr) {
        return false;
    }

    const domain::NodeId nodeId = provider_->id(handle);
    if (!editCallback_(nodeId, value.toString().toStdString())) {
        return false;
    }

    emit dataChanged(index, index);
    return true;
}

void TreeModel::setEditCallback(EditCallback callback) {
    editCallback_ = std::move(callback);
}

void TreeModel::refreshAll() {
    beginResetModel();
    endResetModel();
}

void TreeModel::notifyNodeChanged(NodeId id) {
    const QModelIndex idx = indexForId(id);
    if (idx.isValid()) {
        emit dataChanged(idx, idx);
    }
}

QModelIndex TreeModel::indexForId(NodeId id) const {
    return searchIndex(id, QModelIndex{});
}

QModelIndex TreeModel::searchIndex(NodeId id, const QModelIndex& parent) const {
    const int rows = rowCount(parent);
    for (int row = 0; row < rows; ++row) {
        const QModelIndex current = index(row, 0, parent);
        ITreeDataProvider::NodeHandle handle = handleForIndex(current);
        if (handle != nullptr && provider_->id(handle) == id) {
            return current;
        }
        const QModelIndex found = searchIndex(id, current);
        if (found.isValid()) {
            return found;
        }
    }
    return {};
}

} // namespace micran_tree_cache::presentation