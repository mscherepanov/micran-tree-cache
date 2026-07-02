#include "domain/TreeNode.h"

#include <QtTest/QtTest>
#include <memory>

using micran_tree_cache::domain::NodeId;
using micran_tree_cache::domain::NodeStatus;
using micran_tree_cache::domain::TreeNode;

namespace {
std::unique_ptr<TreeNode> makeNode(std::uint64_t id, NodeStatus status,
                                   std::string payload = "node") {
    return std::make_unique<TreeNode>(NodeId{id}, std::nullopt, std::move(payload), status);
}

} // namespace

class TreeNodeTest : public QObject {
    Q_OBJECT

private slots:
    void attachChildSetsParent();
    void editUnchangedBecomesModified();
    void editNewStaysNew();
    void editDeletedIsRejected();
    void deleteCascadesToDescendants();
    void unloadedChildrenFlagIsIndependent();
};

void TreeNodeTest::attachChildSetsParent() {
    auto root = makeNode(1, NodeStatus::Unchanged);
    TreeNode* child = root->attachChild(makeNode(2, NodeStatus::New));

    QCOMPARE(child->parent(), root.get());
    QCOMPARE(root->children().size(), std::size_t{1});
}

void TreeNodeTest::editUnchangedBecomesModified() {
    auto node = makeNode(1, NodeStatus::Unchanged, "old");

    const bool applied = node->setPayload("new");

    QVERIFY(applied);
    QCOMPARE(node->payload(), std::string{"new"});
    QCOMPARE(node->status(), NodeStatus::Modified);
}

void TreeNodeTest::editNewStaysNew() {
    auto node = makeNode(1, NodeStatus::New, "old");

    QVERIFY(node->setPayload("new"));
    QCOMPARE(node->status(), NodeStatus::New);
}

void TreeNodeTest::editDeletedIsRejected() {
    auto node = makeNode(1, NodeStatus::Deleted, "old");

    const bool applied = node->setPayload("new");

    QVERIFY(!applied);
    QCOMPARE(node->payload(), std::string{"old"});
    QCOMPARE(node->status(), NodeStatus::Deleted);
}

void TreeNodeTest::deleteCascadesToDescendants() {
    auto root = makeNode(1, NodeStatus::Unchanged);
    TreeNode* child = root->attachChild(makeNode(2, NodeStatus::Unchanged));
    TreeNode* grandChild = child->attachChild(makeNode(3, NodeStatus::Unchanged));

    root->markDeletedRecursively();

    QVERIFY(root->isDeleted());
    QVERIFY(child->isDeleted());
    QVERIFY(grandChild->isDeleted());
    QVERIFY(!grandChild->isEditable());
}

void TreeNodeTest::unloadedChildrenFlagIsIndependent() {
    auto node = makeNode(1, NodeStatus::Unchanged);

    QVERIFY(!node->hasUnloadedChildren());
    node->setHasUnloadedChildren(true);

    QVERIFY(node->hasUnloadedChildren());
    QCOMPARE(node->status(), NodeStatus::Unchanged);
}

QTEST_MAIN(TreeNodeTest)
#include "test_tree_node.moc"