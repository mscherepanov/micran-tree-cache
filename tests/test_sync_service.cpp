#include "application/CacheService.h"
#include "application/SyncService.h"
#include "infrastructure/DatabaseRepository.h"
#include "infrastructure/InMemoryDatabase.h"

#include <QtTest/QtTest>

using micran_tree_cache::application::CacheService;
using micran_tree_cache::application::SyncService;
using micran_tree_cache::domain::NodeId;
using micran_tree_cache::domain::NodeStatus;
using micran_tree_cache::infrastructure::DatabaseRepository;
using micran_tree_cache::infrastructure::InMemoryDatabase;

class SyncServiceTest : public QObject {
    Q_OBJECT

private:
    InMemoryDatabase db_;
    std::unique_ptr<DatabaseRepository> repo_;
    std::unique_ptr<CacheService> cache_;

    NodeId rootId() { return db_.fetchRoots().front().id; }

private slots:
    void init();
    void modifiedPayloadPersisted();
    void modifiedBecomesUnchangedAfterSync();
    void newNodePersistedWithRealId();
    void newNodeBecomesUnchangedWithPersistentId();
    void deletedPersistedAndStaysDeleted();
    void nestedNewNodesPersisted();
    void summaryCountsAreCorrect();
};

void SyncServiceTest::init() {
    db_.reset();
    repo_ = std::make_unique<DatabaseRepository>(db_);
    cache_ = std::make_unique<CacheService>(*repo_);
}

void SyncServiceTest::modifiedPayloadPersisted() {
    cache_->loadFromDatabase(rootId());
    cache_->editPayload(rootId(), "Новое имя");

    SyncService sync{*cache_, *repo_};
    sync.apply();

    QCOMPARE(db_.fetchNode(rootId())->payload, std::string{"Новое имя"});
}

void SyncServiceTest::modifiedBecomesUnchangedAfterSync() {
    cache_->loadFromDatabase(rootId());
    cache_->editPayload(rootId(), "Правка");

    SyncService sync{*cache_, *repo_};
    sync.apply();

    QCOMPARE(cache_->findNode(rootId())->status(), NodeStatus::Unchanged);
}

void SyncServiceTest::newNodePersistedWithRealId() {
    cache_->loadFromDatabase(rootId());
    cache_->addChild(rootId(), "Новый потомок");

    const int before = static_cast<int>(db_.fetchChildren(rootId()).size());

    SyncService sync{*cache_, *repo_};
    sync.apply();

    const auto children = db_.fetchChildren(rootId());
    QCOMPARE(static_cast<int>(children.size()), before + 1);

    bool found = false;
    for (const auto& child : children) {
        if (child.payload == "Новый потомок") {
            found = true;
        }
    }
    QVERIFY(found);
}

void SyncServiceTest::newNodeBecomesUnchangedWithPersistentId() {
    cache_->loadFromDatabase(rootId());
    cache_->addChild(rootId(), "Потомок");

    const auto& kids = cache_->findNode(rootId())->children();
    const NodeId temporaryId = kids.back()->id();

    SyncService sync{*cache_, *repo_};
    sync.apply();

    QVERIFY(cache_->findNode(temporaryId) == nullptr);

    const auto& kidsAfter = cache_->findNode(rootId())->children();
    const NodeId persistentId = kidsAfter.back()->id();
    QCOMPARE(kidsAfter.back()->status(), NodeStatus::Unchanged);
    QVERIFY(db_.fetchNode(persistentId).has_value());
}

void SyncServiceTest::deletedPersistedAndStaysDeleted() {
    cache_->loadFromDatabase(rootId());
    const NodeId child = db_.fetchChildren(rootId()).front().id;
    cache_->loadFromDatabase(child);
    cache_->remove(child);

    SyncService sync{*cache_, *repo_};
    sync.apply();

    QVERIFY(db_.fetchNode(child)->deleted);
    QCOMPARE(cache_->findNode(child)->status(), NodeStatus::Deleted);
}

void SyncServiceTest::nestedNewNodesPersisted() {
    cache_->loadFromDatabase(rootId());

    cache_->addChild(rootId(), "Новый родитель");
    const NodeId newParentId = cache_->findNode(rootId())->children().back()->id();
    cache_->addChild(newParentId, "Новый ребёнок");

    SyncService sync{*cache_, *repo_};
    sync.apply();

    const NodeId persistentParentId = cache_->findNode(rootId())->children().back()->id();
    const auto grandChildren = db_.fetchChildren(persistentParentId);
    QCOMPARE(static_cast<int>(grandChildren.size()), 1);
    QCOMPARE(grandChildren.front().payload, std::string{"Новый ребёнок"});
}

void SyncServiceTest::summaryCountsAreCorrect() {
    cache_->loadFromDatabase(rootId());
    const auto children = db_.fetchChildren(rootId());
    cache_->loadFromDatabase(children[0].id);
    cache_->loadFromDatabase(children[1].id);

    cache_->editPayload(rootId(), "Изменён");
    cache_->addChild(rootId(), "Новый");
    cache_->remove(children[1].id);

    SyncService sync{*cache_, *repo_};
    const SyncService::Result result = sync.apply();

    QCOMPARE(result.created, 1);
    QCOMPARE(result.modified, 1);
    QVERIFY(result.deleted >= 1);
    QVERIFY(result.total() >= 3);
}

QTEST_MAIN(SyncServiceTest)
#include "test_sync_service.moc"