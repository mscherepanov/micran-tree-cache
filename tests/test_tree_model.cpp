#include "application/CacheService.h"
#include "infrastructure/DatabaseRepository.h"
#include "infrastructure/InMemoryDatabase.h"
#include "presentation/CacheTreeProvider.h"
#include "presentation/TreeModel.h"

#include <QAbstractItemModelTester>
#include <QtTest/QtTest>
#include <memory>

using micran_tree_cache::application::CacheService;
using micran_tree_cache::domain::NodeId;
using micran_tree_cache::domain::NodeStatus;
using micran_tree_cache::infrastructure::DatabaseRepository;
using micran_tree_cache::infrastructure::InMemoryDatabase;
using micran_tree_cache::presentation::CacheTreeProvider;
using micran_tree_cache::presentation::TreeModel;

class TreeModelTest : public QObject {
    Q_OBJECT

private:
    InMemoryDatabase db_;
    std::unique_ptr<DatabaseRepository> repo_;
    std::unique_ptr<CacheService> cache_;

    NodeId rootId() { return db_.fetchRoots().front().id; }

    std::unique_ptr<TreeModel> makeModel() {
        auto provider = std::make_unique<CacheTreeProvider>(*cache_);
        return std::make_unique<TreeModel>(std::move(provider));
    }

private slots:
    void init();
    void modelPassesQtInvariants();
    void rootRowCountMatchesForest();
    void displayRoleReturnsPayload();
    void statusRoleReflectsNodeStatus();
    void indexForIdFindsNode();
    void childRowsAppearAfterLoad();
    void notEditableWithoutCallback();
    void editableWithCallback();
    void deletedNodeNotEditable();
    void setDataInvokesCallback();
};

void TreeModelTest::init() {
    db_.reset();
    repo_ = std::make_unique<DatabaseRepository>(db_);
    cache_ = std::make_unique<CacheService>(*repo_);
}

void TreeModelTest::modelPassesQtInvariants() {
    cache_->loadFromDatabase(rootId());
    const NodeId child = db_.fetchChildren(rootId()).front().id;
    cache_->loadFromDatabase(child);

    auto model = makeModel();
    model->refreshAll();

    QAbstractItemModelTester tester{model.get(),
                                    QAbstractItemModelTester::FailureReportingMode::Warning};
    QVERIFY(true);
}

void TreeModelTest::rootRowCountMatchesForest() {
    cache_->loadFromDatabase(rootId());
    auto model = makeModel();

    QCOMPARE(model->rowCount(QModelIndex{}), 1);
}

void TreeModelTest::displayRoleReturnsPayload() {
    cache_->loadFromDatabase(rootId());
    auto model = makeModel();

    const QModelIndex rootIndex = model->index(0, 0, QModelIndex{});
    QVERIFY(rootIndex.isValid());
    QCOMPARE(model->data(rootIndex, Qt::DisplayRole).toString(), QString::fromUtf8("Автомобили"));
}

void TreeModelTest::statusRoleReflectsNodeStatus() {
    cache_->loadFromDatabase(rootId());
    cache_->editPayload(rootId(), "Изменено");
    auto model = makeModel();

    const QModelIndex rootIndex = model->index(0, 0, QModelIndex{});
    const auto status =
        static_cast<NodeStatus>(model->data(rootIndex, TreeModel::StatusRole).toInt());
    QCOMPARE(status, NodeStatus::Modified);
}

void TreeModelTest::indexForIdFindsNode() {
    cache_->loadFromDatabase(rootId());
    const NodeId child = db_.fetchChildren(rootId()).front().id;
    cache_->loadFromDatabase(child);
    auto model = makeModel();
    model->refreshAll();

    const QModelIndex childIndex = model->indexForId(child);
    QVERIFY(childIndex.isValid());
    QCOMPARE(static_cast<qulonglong>(micran_tree_cache::domain::toRaw(child)),
             model->data(childIndex, TreeModel::NodeIdRole).toULongLong());
}

void TreeModelTest::childRowsAppearAfterLoad() {
    cache_->loadFromDatabase(rootId());
    auto model = makeModel();

    const QModelIndex rootIndex = model->index(0, 0, QModelIndex{});
    QCOMPARE(model->rowCount(rootIndex), 0);

    const NodeId child = db_.fetchChildren(rootId()).front().id;
    cache_->loadFromDatabase(child);
    model->refreshAll();

    const QModelIndex rootIndexAfter = model->index(0, 0, QModelIndex{});
    QCOMPARE(model->rowCount(rootIndexAfter), 1);
}

void TreeModelTest::notEditableWithoutCallback() {
    cache_->loadFromDatabase(rootId());
    auto model = makeModel();

    const QModelIndex rootIndex = model->index(0, 0, QModelIndex{});
    QVERIFY(!(model->flags(rootIndex) & Qt::ItemIsEditable));
}

void TreeModelTest::editableWithCallback() {
    cache_->loadFromDatabase(rootId());
    auto model = makeModel();
    model->setEditCallback([this](NodeId id, std::string value) {
        return cache_->editPayload(id, value) == CacheService::OperationResult::Success;
    });

    const QModelIndex rootIndex = model->index(0, 0, QModelIndex{});
    QVERIFY(model->flags(rootIndex) & Qt::ItemIsEditable);
}

void TreeModelTest::deletedNodeNotEditable() {
    cache_->loadFromDatabase(rootId());
    cache_->remove(rootId());
    auto model = makeModel();
    model->setEditCallback([](NodeId, std::string) { return true; });

    const QModelIndex rootIndex = model->index(0, 0, QModelIndex{});
    QVERIFY(!(model->flags(rootIndex) & Qt::ItemIsEditable));
}

void TreeModelTest::setDataInvokesCallback() {
    cache_->loadFromDatabase(rootId());
    auto model = makeModel();

    bool called = false;
    model->setEditCallback([&](NodeId id, std::string value) {
        called = true;
        return cache_->editPayload(id, value) == CacheService::OperationResult::Success;
    });

    const QModelIndex rootIndex = model->index(0, 0, QModelIndex{});
    const bool ok = model->setData(rootIndex, QString::fromUtf8("Новое"), Qt::EditRole);

    QVERIFY(ok);
    QVERIFY(called);
    QCOMPARE(cache_->findNode(rootId())->payload(), std::string{"Новое"});
}

QTEST_MAIN(TreeModelTest)
#include "test_tree_model.moc"