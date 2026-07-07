#include "presentation/mainwindow.h"

#include "application/CacheService.h"
#include "application/SyncService.h"
#include "domain/IDatabaseRepository.h"
#include "presentation/CacheTreeProvider.h"
#include "presentation/DatabaseTreeProvider.h"
#include "presentation/NodeItemDelegate.h"
#include "presentation/TreeModel.h"

#include <QAction>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QInputDialog>
#include <QLabel>
#include <QMenu>
#include <QMessageBox>
#include <QPushButton>
#include <QSplitter>
#include <QStatusBar>
#include <QTreeView>
#include <QVBoxLayout>
#include <QWidget>
#include <memory>

namespace micran_tree_cache::presentation {

using application::CacheService;
using application::SyncService;
using domain::NodeId;

namespace {

QString describeResult(CacheService::OperationResult result) {
    switch (result) {
    case CacheService::OperationResult::Success:
        return QObject::tr("Готово");
    case CacheService::OperationResult::NodeNotFound:
        return QObject::tr("Элемент не найден");
    case CacheService::OperationResult::NodeDeleted:
        return QObject::tr("Элемент удалён и недоступен для изменения");
    case CacheService::OperationResult::AlreadyInCache:
        return QObject::tr("Элемент уже загружен в кэш");
    }
    return {};
}

} // namespace

MainWindow::MainWindow(CacheService& cache, domain::IDatabaseRepository& repository,
                       QWidget* parent)
    : QMainWindow{parent}, cache_{cache}, repository_{repository} {
    buildUi();
    connectActions();
    refreshViews();
    updateActionStates();
    setWindowTitle(tr("micran-tree-cache"));
    resize(900, 600);
}

MainWindow::~MainWindow() = default;

void MainWindow::buildUi() {
    auto dbProvider = std::make_unique<DatabaseTreeProvider>(repository_);
    databaseProvider_ = dbProvider.get();
    databaseModel_ = new TreeModel(std::move(dbProvider), this);

    auto cacheProvider = std::make_unique<CacheTreeProvider>(cache_);
    cacheModel_ = new TreeModel(std::move(cacheProvider), this);

    cacheModel_->setEditCallback([this](domain::NodeId id, std::string value) {
        const auto result = cache_.editPayload(id, std::move(value));
        const bool ok = result == CacheService::OperationResult::Success;
        if (ok) {
            showStatus(tr("Элемент изменён"));
        } else {
            showStatus(describeResult(result));
        }
        return ok;
    });

    databaseView_ = new QTreeView(this);
    databaseView_->setModel(databaseModel_);
    databaseView_->setItemDelegate(new NodeItemDelegate(databaseView_));
    databaseView_->setHeaderHidden(false);
    databaseView_->header()->setSectionResizeMode(QHeaderView::Stretch);
    databaseView_->setContextMenuPolicy(Qt::CustomContextMenu);
    databaseView_->setEditTriggers(QAbstractItemView::NoEditTriggers);

    cacheView_ = new QTreeView(this);
    cacheView_->setModel(cacheModel_);
    cacheView_->setItemDelegate(new NodeItemDelegate(cacheView_));
    cacheView_->setHeaderHidden(false);
    cacheView_->header()->setSectionResizeMode(QHeaderView::Stretch);
    cacheView_->setContextMenuPolicy(Qt::CustomContextMenu);
    cacheView_->setEditTriggers(QAbstractItemView::DoubleClicked |
                                QAbstractItemView::EditKeyPressed);

    loadButton_ = new QPushButton(tr("Загрузить в кэш"), this);

    auto* dbBox = new QGroupBox(tr("База данных"), this);
    auto* dbLayout = new QVBoxLayout(dbBox);
    dbLayout->addWidget(databaseView_);
    dbLayout->addWidget(loadButton_);

    addButton_ = new QPushButton(tr("Добавить дочерний"), this);
    editButton_ = new QPushButton(tr("Изменить"), this);
    removeButton_ = new QPushButton(tr("Удалить"), this);
    applyButton_ = new QPushButton(tr("Сохранить в БД"), this);

    auto* cacheButtons = new QHBoxLayout;
    cacheButtons->addWidget(addButton_);
    cacheButtons->addWidget(editButton_);
    cacheButtons->addWidget(removeButton_);
    cacheButtons->addStretch();
    cacheButtons->addWidget(applyButton_);

    auto* cacheBox = new QGroupBox(tr("Локальный кэш"), this);
    auto* cacheLayout = new QVBoxLayout(cacheBox);
    cacheLayout->addWidget(cacheView_);
    cacheLayout->addLayout(cacheButtons);

    auto* splitter = new QSplitter(Qt::Horizontal, this);
    splitter->addWidget(dbBox);
    splitter->addWidget(cacheBox);
    splitter->setStretchFactor(0, 1);
    splitter->setStretchFactor(1, 1);

    resetButton_ = new QPushButton(tr("Сбросить приложение"), this);

    auto* bottomBar = new QHBoxLayout;
    bottomBar->addStretch();
    bottomBar->addWidget(resetButton_);

    auto* central = new QWidget(this);
    auto* rootLayout = new QVBoxLayout(central);
    rootLayout->addWidget(splitter, 1);
    rootLayout->addLayout(bottomBar);
    setCentralWidget(central);

    statusBar()->showMessage(tr("Готово к работе"));
}

void MainWindow::connectActions() {
    connect(loadButton_, &QPushButton::clicked, this, &MainWindow::onLoadToCache);
    connect(addButton_, &QPushButton::clicked, this, &MainWindow::onAddChild);
    connect(editButton_, &QPushButton::clicked, this, &MainWindow::onEditPayload);
    connect(removeButton_, &QPushButton::clicked, this, &MainWindow::onRemove);
    connect(applyButton_, &QPushButton::clicked, this, &MainWindow::onApplyToDatabase);
    connect(resetButton_, &QPushButton::clicked, this, &MainWindow::onReset);

    connect(databaseView_->selectionModel(), &QItemSelectionModel::selectionChanged, this,
            [this] { updateActionStates(); });
    connect(cacheView_->selectionModel(), &QItemSelectionModel::selectionChanged, this,
            [this] { updateActionStates(); });

    connect(databaseView_, &QWidget::customContextMenuRequested, this,
            &MainWindow::onDatabaseContextMenu);
    connect(cacheView_, &QWidget::customContextMenuRequested, this,
            &MainWindow::onCacheContextMenu);
}

std::optional<NodeId> MainWindow::selectedId(const QTreeView* view) const {
    const QModelIndex index = view->currentIndex();
    if (!index.isValid()) {
        return std::nullopt;
    }
    const auto raw = index.data(TreeModel::NodeIdRole).toULongLong();
    return NodeId{raw};
}

void MainWindow::onLoadToCache() {
    const auto id = selectedId(databaseView_);
    if (!id.has_value()) {
        showStatus(tr("Выберите элемент в базе данных"));
        return;
    }

    const auto result = cache_.loadFromDatabase(*id);

    cacheModel_->refreshAll();
    cacheView_->expandAll();
    updateActionStates();
    showStatus(describeResult(result));
}

void MainWindow::onAddChild() {
    const auto id = selectedId(cacheView_);
    if (!id.has_value()) {
        showStatus(tr("Выберите родительский элемент в кэше"));
        return;
    }

    bool ok = false;
    const QString text =
        QInputDialog::getText(this, tr("Новый дочерний элемент"), tr("Строковое поле:"),
                              QLineEdit::Normal, QString{}, &ok);
    if (!ok) {
        return;
    }

    const auto result = cache_.addChild(*id, text.toStdString());
    cacheModel_->refreshAll();
    cacheView_->expandAll();
    updateActionStates();
    showStatus(describeResult(result));
}

void MainWindow::onEditPayload() {
    const auto id = selectedId(cacheView_);
    if (!id.has_value()) {
        showStatus(tr("Выберите элемент в кэше"));
        return;
    }

    const QModelIndex index = cacheView_->currentIndex();
    const QString current = index.data(Qt::DisplayRole).toString();

    bool ok = false;
    const QString text = QInputDialog::getText(
        this, tr("Изменение элемента"), tr("Строковое поле:"), QLineEdit::Normal, current, &ok);
    if (!ok) {
        return;
    }

    const auto result = cache_.editPayload(*id, text.toStdString());
    if (result == CacheService::OperationResult::Success) {
        cacheModel_->notifyNodeChanged(*id);
    }
    updateActionStates();
    showStatus(describeResult(result));
}

void MainWindow::onRemove() {
    const auto id = selectedId(cacheView_);
    if (!id.has_value()) {
        showStatus(tr("Выберите элемент в кэше"));
        return;
    }

    const QString name = cacheView_->currentIndex().data(Qt::DisplayRole).toString();
    const auto answer = QMessageBox::question(
        this, tr("Удаление элемента"),
        tr("Удалить элемент «%1» вместе со всеми дочерними элементами?").arg(name),
        QMessageBox::Yes | QMessageBox::No);
    if (answer != QMessageBox::Yes) {
        return;
    }

    const auto result = cache_.remove(*id);

    cacheModel_->refreshAll();
    cacheView_->expandAll();
    updateActionStates();
    showStatus(describeResult(result));
}

void MainWindow::onApplyToDatabase() {
    if (cache_.isEmpty()) {
        showStatus(tr("Кэш пуст — нечего сохранять"));
        return;
    }

    SyncService sync{cache_, repository_};
    const SyncService::Result result = sync.apply();

    databaseProvider_->rebuild();
    refreshViews();
    updateActionStates();

    showStatus(tr("Сохранено в БД: создано %1, изменено %2, удалено %3")
                   .arg(result.created)
                   .arg(result.modified)
                   .arg(result.deleted));
}

void MainWindow::onReset() {
    const auto answer = QMessageBox::question(
        this, tr("Сброс приложения"), tr("Сбросить все изменения и вернуть исходное состояние?"),
        QMessageBox::Yes | QMessageBox::No);
    if (answer != QMessageBox::Yes) {
        return;
    }

    cache_.clear();
    databaseProvider_->rebuild();
    refreshViews();
    updateActionStates();
    showStatus(tr("Приложение сброшено в исходное состояние"));
}

void MainWindow::onDatabaseContextMenu(const QPoint& pos) {
    const QModelIndex index = databaseView_->indexAt(pos);
    if (!index.isValid()) {
        return;
    }
    databaseView_->setCurrentIndex(index);

    QMenu menu{this};
    menu.addAction(tr("Загрузить в кэш"), this, &MainWindow::onLoadToCache);
    menu.exec(databaseView_->viewport()->mapToGlobal(pos));
}

void MainWindow::onCacheContextMenu(const QPoint& pos) {
    const QModelIndex index = cacheView_->indexAt(pos);
    if (!index.isValid()) {
        return;
    }
    cacheView_->setCurrentIndex(index);

    const auto status = static_cast<domain::NodeStatus>(index.data(TreeModel::StatusRole).toInt());
    const bool deleted = status == domain::NodeStatus::Deleted;

    QMenu menu{this};
    QAction* add = menu.addAction(tr("Добавить дочерний"), this, &MainWindow::onAddChild);
    QAction* edit = menu.addAction(tr("Изменить"), this, &MainWindow::onEditPayload);
    menu.addSeparator();
    menu.addAction(tr("Удалить"), this, &MainWindow::onRemove);

    add->setEnabled(!deleted);
    edit->setEnabled(!deleted);

    menu.exec(cacheView_->viewport()->mapToGlobal(pos));
}

void MainWindow::updateActionStates() {
    const bool dbSelected = selectedId(databaseView_).has_value();
    const bool cacheSelected = selectedId(cacheView_).has_value();

    loadButton_->setEnabled(dbSelected);
    addButton_->setEnabled(cacheSelected);
    editButton_->setEnabled(cacheSelected);
    removeButton_->setEnabled(cacheSelected);
    applyButton_->setEnabled(!cache_.isEmpty());
}

void MainWindow::refreshViews() {
    databaseModel_->refreshAll();
    cacheModel_->refreshAll();
    databaseView_->expandAll();
    cacheView_->expandAll();
}

void MainWindow::showStatus(const QString& message) {
    statusBar()->showMessage(message, 5000);
}

} // namespace micran_tree_cache::presentation