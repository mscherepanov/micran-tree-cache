#include "infrastructure/InMemoryDatabase.h"

#include <QtTest/QtTest>
#include <algorithm>
#include <functional>

using micran_tree_cache::domain::ChangeSet;
using micran_tree_cache::domain::NewNodeChange;
using micran_tree_cache::domain::NodeId;
using micran_tree_cache::domain::NodeSnapshot;
using micran_tree_cache::infrastructure::InMemoryDatabase;

namespace {

int treeDepth(const InMemoryDatabase& db, NodeId id) {
    const auto children = db.fetchChildren(id);
    int maxChild = 0;
    for (const auto& child : children) {
        maxChild = std::max(maxChild, treeDepth(db, child.id));
    }
    return 1 + maxChild;
}

} // namespace

class InMemoryDatabaseTest : public QObject {
    Q_OBJECT

private slots:
    void hasSingleRoot();
    void testDataDepthAtLeastFour();
    void fetchNodeReturnsSnapshot();
    void fetchMissingNodeReturnsNullopt();
    void hasChildrenFlagIsAccurate();
    void applyCreatesNewNodeWithPersistentId();
    void applyModifiesPayload();
    void applyDeleteCascadesToDescendants();
    void resetRestoresInitialState();
};

void InMemoryDatabaseTest::hasSingleRoot() {
    const InMemoryDatabase db;
    const auto roots = db.fetchRoots();

    QCOMPARE(roots.size(), std::size_t{1});
    QVERIFY(!roots.front().parentId.has_value());
}

void InMemoryDatabaseTest::testDataDepthAtLeastFour() {
    const InMemoryDatabase db;
    const auto roots = db.fetchRoots();
    QVERIFY(!roots.empty());

    QVERIFY(treeDepth(db, roots.front().id) >= 4);
}

void InMemoryDatabaseTest::fetchNodeReturnsSnapshot() {
    const InMemoryDatabase db;
    const auto roots = db.fetchRoots();
    const NodeId rootId = roots.front().id;

    const auto snapshot = db.fetchNode(rootId);

    QVERIFY(snapshot.has_value());
    QCOMPARE(snapshot->id, rootId);
    QVERIFY(!snapshot->deleted);
}

void InMemoryDatabaseTest::fetchMissingNodeReturnsNullopt() {
    const InMemoryDatabase db;
    const auto snapshot = db.fetchNode(NodeId{9999});
    QVERIFY(!snapshot.has_value());
}

void InMemoryDatabaseTest::hasChildrenFlagIsAccurate() {
    const InMemoryDatabase db;
    const auto roots = db.fetchRoots();
    const auto rootChildren = db.fetchChildren(roots.front().id);
    QVERIFY(!rootChildren.empty());

    QVERIFY(db.fetchNode(roots.front().id)->hasChildren);

    const auto isLeaf = [&db](const NodeSnapshot& s) { return db.fetchChildren(s.id).empty(); };
    std::vector<NodeSnapshot> frontier = rootChildren;
    std::optional<NodeSnapshot> leaf;
    while (!frontier.empty() && !leaf.has_value()) {
        std::vector<NodeSnapshot> next;
        for (const auto& node : frontier) {
            if (isLeaf(node)) {
                leaf = node;
                break;
            }
            const auto ch = db.fetchChildren(node.id);
            next.insert(next.end(), ch.begin(), ch.end());
        }
        frontier = next;
    }
    QVERIFY(leaf.has_value());
    QVERIFY(!db.fetchNode(leaf->id)->hasChildren);
}

void InMemoryDatabaseTest::applyCreatesNewNodeWithPersistentId() {
    InMemoryDatabase db;
    const auto roots = db.fetchRoots();
    const NodeId parentId = roots.front().id;

    ChangeSet changes;
    const NodeId temporaryId{100000};
    changes.created.push_back(NewNodeChange{temporaryId, parentId, "Новый узел"});

    const auto result = db.applyChanges(changes);

    QCOMPARE(result.assignedIds.size(), std::size_t{1});
    QCOMPARE(result.assignedIds.front().temporaryId, temporaryId);

    const NodeId persistentId = result.assignedIds.front().persistentId;
    const auto snapshot = db.fetchNode(persistentId);
    QVERIFY(snapshot.has_value());
    QCOMPARE(snapshot->payload, std::string{"Новый узел"});
    QCOMPARE(snapshot->parentId, std::optional<NodeId>{parentId});
}

void InMemoryDatabaseTest::applyModifiesPayload() {
    InMemoryDatabase db;
    const auto roots = db.fetchRoots();
    const NodeId rootId = roots.front().id;

    ChangeSet changes;
    changes.modified.push_back({rootId, "Изменённое значение"});
    db.applyChanges(changes);

    QCOMPARE(db.fetchNode(rootId)->payload, std::string{"Изменённое значение"});
}

void InMemoryDatabaseTest::applyDeleteCascadesToDescendants() {
    InMemoryDatabase db;
    const auto roots = db.fetchRoots();
    const NodeId rootChild = db.fetchChildren(roots.front().id).front().id;

    ChangeSet changes;
    changes.deleted.push_back(rootChild);
    db.applyChanges(changes);

    QVERIFY(db.fetchNode(rootChild)->deleted);

    const std::function<void(NodeId)> checkDeleted = [&](NodeId id) {
        QVERIFY(db.fetchNode(id)->deleted);
        for (const auto& child : db.fetchChildren(id)) {
            checkDeleted(child.id);
        }
    };
    for (const auto& child : db.fetchChildren(rootChild)) {
        checkDeleted(child.id);
    }
}

void InMemoryDatabaseTest::resetRestoresInitialState() {
    InMemoryDatabase db;
    const auto rootsBefore = db.fetchRoots();
    const NodeId rootId = rootsBefore.front().id;

    ChangeSet changes;
    changes.deleted.push_back(rootId);
    db.applyChanges(changes);
    QVERIFY(db.fetchNode(rootId)->deleted);

    db.reset();

    const auto rootsAfter = db.fetchRoots();
    QCOMPARE(rootsAfter.size(), std::size_t{1});
    QVERIFY(!db.fetchNode(rootsAfter.front().id)->deleted);
}

QTEST_MAIN(InMemoryDatabaseTest)
#include "test_in_memory_database.moc"