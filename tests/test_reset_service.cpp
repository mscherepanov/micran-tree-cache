#include "application/CacheService.h"
#include "application/ResetService.h"
#include "application/SyncService.h"
#include "infrastructure/DatabaseRepository.h"
#include "infrastructure/InMemoryDatabase.h"

#include <QtTest/QtTest>

using micran_tree_cache::application::CacheService;
using micran_tree_cache::application::ResetService;
using micran_tree_cache::application::SyncService;
using micran_tree_cache::domain::NodeId;
using micran_tree_cache::infrastructure::DatabaseRepository;
using micran_tree_cache::infrastructure::InMemoryDatabase;

class ResetServiceTest : public QObject {
    Q_OBJECT

private:
    InMemoryDatabase db_;
    std::unique_ptr<DatabaseRepository> repo_;
    std::unique_ptr<CacheService> cache_;

    NodeId rootId() { return db_.fetchRoots().front().id; }

private slots:
    void init();
    void clearsCache();
    void restoresDatabaseAfterAppliedChanges();
    void fullCycleReturnsToInitialState();
};

void ResetServiceTest::init() {
    db_.reset();
    repo_ = std::make_unique<DatabaseRepository>(db_);
    cache_ = std::make_unique<CacheService>(*repo_);
}

void ResetServiceTest::clearsCache() {
    cache_->loadFromDatabase(rootId());
    QVERIFY(!cache_->isEmpty());

    ResetService reset{*cache_, db_};
    reset.reset();

    QVERIFY(cache_->isEmpty());
}

void ResetServiceTest::restoresDatabaseAfterAppliedChanges() {
    const std::string originalPayload = db_.fetchNode(rootId())->payload;

    cache_->loadFromDatabase(rootId());
    cache_->editPayload(rootId(), "Изменённое значение");
    SyncService sync{*cache_, *repo_};
    sync.apply();
    QCOMPARE(db_.fetchNode(rootId())->payload, std::string{"Изменённое значение"});

    ResetService reset{*cache_, db_};
    reset.reset();
    QCOMPARE(db_.fetchNode(rootId())->payload, originalPayload);
}

void ResetServiceTest::fullCycleReturnsToInitialState() {
    const int originalRootChildren = static_cast<int>(db_.fetchChildren(rootId()).size());
    const NodeId childToDelete = db_.fetchChildren(rootId()).front().id;

    cache_->loadFromDatabase(rootId());
    cache_->loadFromDatabase(childToDelete);
    cache_->addChild(rootId(), "Новый узел");
    cache_->remove(childToDelete);
    SyncService sync{*cache_, *repo_};
    sync.apply();

    QVERIFY(db_.fetchNode(childToDelete)->deleted);

    ResetService reset{*cache_, db_};
    reset.reset();

    QCOMPARE(static_cast<int>(db_.fetchChildren(rootId()).size()), originalRootChildren);
    QVERIFY(!db_.fetchNode(childToDelete)->deleted);
    QVERIFY(cache_->isEmpty());
}

QTEST_MAIN(ResetServiceTest)
#include "test_reset_service.moc"