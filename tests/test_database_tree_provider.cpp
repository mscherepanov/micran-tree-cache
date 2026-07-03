#include "infrastructure/DatabaseRepository.h"
#include "infrastructure/InMemoryDatabase.h"
#include "presentation/DatabaseTreeProvider.h"

#include <QtTest/QtTest>
#include <functional>

using micran_tree_cache::domain::ChangeSet;
using micran_tree_cache::domain::NodeId;
using micran_tree_cache::domain::NodeStatus;
using micran_tree_cache::infrastructure::DatabaseRepository;
using micran_tree_cache::infrastructure::InMemoryDatabase;
using micran_tree_cache::presentation::DatabaseTreeProvider;
using NodeHandle = micran_tree_cache::presentation::ITreeDataProvider::NodeHandle;

class DatabaseTreeProviderTest : public QObject {
    Q_OBJECT

private:
    InMemoryDatabase db_;
    std::unique_ptr<DatabaseRepository> repo_;

    static int countNodes(const DatabaseTreeProvider& provider) {
        std::function<int(NodeHandle)> countSubtree = [&](NodeHandle node) {
            int total = 1;
            const int children = provider.childCount(node);
            for (int i = 0; i < children; ++i) {
                total += countSubtree(provider.childAt(node, i));
            }
            return total;
        };

        int total = 0;
        for (int i = 0; i < provider.rootCount(); ++i) {
            total += countSubtree(provider.rootAt(i));
        }
        return total;
    }

    static int depth(const DatabaseTreeProvider& provider, NodeHandle node) {
        int maxChild = 0;
        const int children = provider.childCount(node);
        for (int i = 0; i < children; ++i) {
            maxChild = std::max(maxChild, depth(provider, provider.childAt(node, i)));
        }
        return 1 + maxChild;
    }

private slots:
    void init();
    void materializesSingleRoot();
    void structureMatchesDatabase();
    void depthAtLeastFour();
    void parentAndIndexAreConsistent();
    void payloadMatchesDatabase();
    void deletedStatusReflected();
    void rebuildPicksUpChanges();
};

void DatabaseTreeProviderTest::init() {
    db_.reset();
    repo_ = std::make_unique<DatabaseRepository>(db_);
}

void DatabaseTreeProviderTest::materializesSingleRoot() {
    const DatabaseTreeProvider provider{*repo_};
    QCOMPARE(provider.rootCount(), 1);
    QCOMPARE(provider.parentOf(provider.rootAt(0)), static_cast<NodeHandle>(nullptr));
}

void DatabaseTreeProviderTest::structureMatchesDatabase() {
    const DatabaseTreeProvider provider{*repo_};

    std::function<int(NodeId)> countDb = [&](NodeId id) {
        int total = 1;
        for (const auto& child : db_.fetchChildren(id)) {
            total += countDb(child.id);
        }
        return total;
    };
    int dbTotal = 0;
    for (const auto& root : db_.fetchRoots()) {
        dbTotal += countDb(root.id);
    }

    QCOMPARE(countNodes(provider), dbTotal);
}

void DatabaseTreeProviderTest::depthAtLeastFour() {
    const DatabaseTreeProvider provider{*repo_};
    QVERIFY(depth(provider, provider.rootAt(0)) >= 4);
}

void DatabaseTreeProviderTest::parentAndIndexAreConsistent() {
    const DatabaseTreeProvider provider{*repo_};
    NodeHandle root = provider.rootAt(0);

    const int children = provider.childCount(root);
    QVERIFY(children > 0);
    for (int i = 0; i < children; ++i) {
        NodeHandle child = provider.childAt(root, i);
        QCOMPARE(provider.parentOf(child), root);
        QCOMPARE(provider.indexOfChild(child), i);
    }
    QCOMPARE(provider.indexOfChild(root), 0);
}

void DatabaseTreeProviderTest::payloadMatchesDatabase() {
    const DatabaseTreeProvider provider{*repo_};
    NodeHandle root = provider.rootAt(0);

    const NodeId rootId = provider.id(root);
    const auto snapshot = db_.fetchNode(rootId);
    QVERIFY(snapshot.has_value());
    QCOMPARE(provider.payload(root), QString::fromStdString(snapshot->payload));
}

void DatabaseTreeProviderTest::deletedStatusReflected() {
    const NodeId rootId = db_.fetchRoots().front().id;
    const NodeId childId = db_.fetchChildren(rootId).front().id;

    ChangeSet changes;
    changes.deleted.push_back(childId);
    db_.applyChanges(changes);

    const DatabaseTreeProvider provider{*repo_};
    NodeHandle root = provider.rootAt(0);

    NodeHandle target = nullptr;
    for (int i = 0; i < provider.childCount(root); ++i) {
        if (provider.id(provider.childAt(root, i)) == childId) {
            target = provider.childAt(root, i);
            break;
        }
    }
    QVERIFY(target != nullptr);
    QCOMPARE(provider.status(target), NodeStatus::Deleted);
}

void DatabaseTreeProviderTest::rebuildPicksUpChanges() {
    DatabaseTreeProvider provider{*repo_};
    const int before = countNodes(provider);

    const NodeId rootId = db_.fetchRoots().front().id;
    ChangeSet changes;
    changes.created.push_back({NodeId{500000}, rootId, "Добавленный"});
    db_.applyChanges(changes);

    provider.rebuild();
    QCOMPARE(countNodes(provider), before + 1);
}

QTEST_MAIN(DatabaseTreeProviderTest)
#include "test_database_tree_provider.moc"