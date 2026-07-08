#ifndef MICRAN_TREE_CACHE_PRESENTATION_MAIN_WINDOW_H
#define MICRAN_TREE_CACHE_PRESENTATION_MAIN_WINDOW_H

#include "domain/NodeId.h"

#include <QMainWindow>
#include <optional>

class QTreeView;
class QPushButton;
class QLabel;

namespace micran_tree_cache::application {
class CacheService;
}

namespace micran_tree_cache::domain {
class IDatabaseRepository;
class IDatabaseResetter;
} // namespace micran_tree_cache::domain

namespace micran_tree_cache::presentation {

class TreeModel;
class DatabaseTreeProvider;

class MainWindow final : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(application::CacheService& cache, domain::IDatabaseRepository& repository,
               domain::IDatabaseResetter& resetter, QWidget* parent = nullptr);
    ~MainWindow() override;

private slots:
    void onLoadToCache();
    void onAddChild();
    void onEditPayload();
    void onRemove();
    void onApplyToDatabase();
    void onReset();

    void onDatabaseContextMenu(const QPoint& pos);
    void onCacheContextMenu(const QPoint& pos);

private:
    void buildUi();
    void connectActions();

    [[nodiscard]] std::optional<domain::NodeId> selectedId(const QTreeView* view) const;

    void updateActionStates();
    void refreshViews();
    void showStatus(const QString& message);

    application::CacheService& cache_;
    domain::IDatabaseRepository& repository_;
    domain::IDatabaseResetter& resetter_;

    QTreeView* databaseView_{nullptr};
    QTreeView* cacheView_{nullptr};

    TreeModel* databaseModel_{nullptr};
    TreeModel* cacheModel_{nullptr};
    DatabaseTreeProvider* databaseProvider_{nullptr};

    QPushButton* loadButton_{nullptr};
    QPushButton* addButton_{nullptr};
    QPushButton* editButton_{nullptr};
    QPushButton* removeButton_{nullptr};
    QPushButton* applyButton_{nullptr};
    QPushButton* resetButton_{nullptr};
};

} // namespace micran_tree_cache::presentation

#endif // MICRAN_TREE_CACHE_PRESENTATION_MAIN_WINDOW_H