#include "application/CacheService.h"
#include "infrastructure/DatabaseRepository.h"
#include "infrastructure/InMemoryDatabase.h"

#include <QtTest/QtTest>

using micran_tree_cache::application::CacheService;
using micran_tree_cache::domain::NodeId;
using micran_tree_cache::domain::NodeSnapshot;
using micran_tree_cache::domain::NodeStatus;
using micran_tree_cache::infrastructure::DatabaseRepository;
using micran_tree_cache::infrastructure::InMemoryDatabase;

using OperationResult = CacheService::OperationResult;

class CacheServiceTest : public QObject {
    Q_OBJECT

private:
    static NodeId rootId(InMemoryDatabase& db) { return db.fetchRoots().front().id; }

private slots:
    void loadRootAddsToForest();
    void loadMissingNodeReturnsNotFound();
    void loadChildMergesUnderExistingParent();
    void loadParentAdoptsExistingOrphan();
    void reloadKeepsLocalChanges();
    void addChildCreatesNewNode();
    void addChildUnderDeletedIsRejected();
    void editChangesStatusToModified();
    void editDeletedIsRejected();
    void removeCascadesToChildren();
    void unloadedFlagSetOnLoadClearedOnExpand();
    void clearEmptiesCache();
};

void CacheServiceTest::loadRootAddsToForest() {
    InMemoryDatabase db;
    DatabaseRepository repo{db};
    CacheService cache{repo};

    const NodeId root = rootId(db);
    QCOMPARE(cache.loadFromDatabase(root), OperationResult::Success);

    QCOMPARE(cache.roots().size(), std::size_t{1});
    QCOMPARE(cache.findNode(root)->status(), NodeStatus::Unchanged);
}

void CacheServiceTest::loadMissingNodeReturnsNotFound() {
    InMemoryDatabase db;
    DatabaseRepository repo{db};
    CacheService cache{repo};

    QCOMPARE(cache.loadFromDatabase(NodeId{999999}), OperationResult::NodeNotFound);
}

void CacheServiceTest::loadChildMergesUnderExistingParent() {
    InMemoryDatabase db;
    DatabaseRepository repo{db};
    CacheService cache{repo};

    const NodeId root = rootId(db);
    const NodeId child = db.fetchChildren(root).front().id;

    cache.loadFromDatabase(root);
    cache.loadFromDatabase(child);

    QCOMPARE(cache.roots().size(), std::size_t{1});
    QCOMPARE(cache.findNode(child)->parent(), cache.findNode(root));
}

void CacheServiceTest::loadParentAdoptsExistingOrphan() {
    InMemoryDatabase db;
    DatabaseRepository repo{db};
    CacheService cache{repo};

    const NodeId root = rootId(db);
    const NodeId child = db.fetchChildren(root).front().id;

    cache.loadFromDatabase(child);
    QCOMPARE(cache.roots().size(), std::size_t{1});

    cache.loadFromDatabase(root);

    QCOMPARE(cache.roots().size(), std::size_t{1});
    QCOMPARE(cache.findNode(child)->parent(), cache.findNode(root));
}

void CacheServiceTest::reloadKeepsLocalChanges() {
    InMemoryDatabase db;
    DatabaseRepository repo{db};
    CacheService cache{repo};

    const NodeId root = rootId(db);
    cache.loadFromDatabase(root);
    cache.editPayload(root, "Локальное изменение");

    QCOMPARE(cache.loadFromDatabase(root), OperationResult::AlreadyInCache);
    QCOMPARE(cache.findNode(root)->payload(), std::string{"Локальное изменение"});
    QCOMPARE(cache.findNode(root)->status(), NodeStatus::Modified);
}

void CacheServiceTest::addChildCreatesNewNode() {
    InMemoryDatabase db;
    DatabaseRepository repo{db};
    CacheService cache{repo};

    const NodeId root = rootId(db);
    cache.loadFromDatabase(root);

    const std::size_t before = cache.findNode(root)->children().size();
    QCOMPARE(cache.addChild(root, "Новый"), OperationResult::Success);
    QCOMPARE(cache.findNode(root)->children().size(), before + 1);

    const auto& lastChild = cache.findNode(root)->children().back();
    QCOMPARE(lastChild->status(), NodeStatus::New);
}

void CacheServiceTest::addChildUnderDeletedIsRejected() {
    InMemoryDatabase db;
    DatabaseRepository repo{db};
    CacheService cache{repo};

    const NodeId root = rootId(db);
    cache.loadFromDatabase(root);
    cache.remove(root);

    QCOMPARE(cache.addChild(root, "Нельзя"), OperationResult::NodeDeleted);
}

void CacheServiceTest::editChangesStatusToModified() {
    InMemoryDatabase db;
    DatabaseRepository repo{db};
    CacheService cache{repo};

    const NodeId root = rootId(db);
    cache.loadFromDatabase(root);

    QCOMPARE(cache.editPayload(root, "Правка"), OperationResult::Success);
    QCOMPARE(cache.findNode(root)->status(), NodeStatus::Modified);
}

void CacheServiceTest::editDeletedIsRejected() {
    InMemoryDatabase db;
    DatabaseRepository repo{db};
    CacheService cache{repo};

    const NodeId root = rootId(db);
    cache.loadFromDatabase(root);
    cache.remove(root);

    QCOMPARE(cache.editPayload(root, "Правка"), OperationResult::NodeDeleted);
}

void CacheServiceTest::removeCascadesToChildren() {
    InMemoryDatabase db;
    DatabaseRepository repo{db};
    CacheService cache{repo};

    const NodeId root = rootId(db);
    const NodeId child = db.fetchChildren(root).front().id;
    cache.loadFromDatabase(root);
    cache.loadFromDatabase(child);

    cache.remove(root);

    QVERIFY(cache.findNode(root)->isDeleted());
    QVERIFY(cache.findNode(child)->isDeleted());
}

void CacheServiceTest::unloadedFlagSetOnLoadClearedOnExpand() {
    InMemoryDatabase db;
    DatabaseRepository repo{db};
    CacheService cache{repo};

    const NodeId root = rootId(db);
    cache.loadFromDatabase(root);

    QVERIFY(cache.findNode(root)->hasUnloadedChildren());

    const NodeId child = db.fetchChildren(root).front().id;
    cache.loadFromDatabase(child);
    QVERIFY(!cache.findNode(root)->hasUnloadedChildren());
}

void CacheServiceTest::clearEmptiesCache() {
    InMemoryDatabase db;
    DatabaseRepository repo{db};
    CacheService cache{repo};

    cache.loadFromDatabase(rootId(db));
    QVERIFY(!cache.isEmpty());

    cache.clear();
    QVERIFY(cache.isEmpty());
    QCOMPARE(cache.roots().size(), std::size_t{0});
}

QTEST_MAIN(CacheServiceTest)
#include "test_cache_service.moc"