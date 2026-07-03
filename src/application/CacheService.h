#ifndef MICRAN_TREE_CACHE_APPLICATION_CACHE_SERVICE_H
#define MICRAN_TREE_CACHE_APPLICATION_CACHE_SERVICE_H

#include "domain/IDatabaseRepository.h"
#include "domain/NodeId.h"
#include "domain/TreeNode.h"

#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

namespace micran_tree_cache::application {
class CacheService {
public:
    enum class OperationResult : std::uint8_t {
        Success,
        NodeNotFound,
        NodeDeleted,
        AlreadyInCache,
    };

    explicit CacheService(domain::IDatabaseRepository& repository);

    OperationResult loadFromDatabase(domain::NodeId id);
    OperationResult addChild(domain::NodeId parentId, std::string payload);
    OperationResult editPayload(domain::NodeId id, std::string payload);
    OperationResult remove(domain::NodeId id);

    void clear();

    [[nodiscard]] const std::vector<std::unique_ptr<domain::TreeNode>>& roots() const noexcept {
        return roots_;
    }
    [[nodiscard]] domain::TreeNode* findNode(domain::NodeId id) const;
    [[nodiscard]] bool isEmpty() const noexcept { return roots_.empty(); }

private:
    domain::TreeNode* insertLoadedNode(const domain::NodeSnapshot& snapshot);

    void adoptOrphans(domain::TreeNode* node);
    void refreshUnloadedFlag(domain::TreeNode* node);

    std::unique_ptr<domain::TreeNode> detachRoot(domain::NodeId id);

    [[nodiscard]] domain::NodeId nextTemporaryId() noexcept;

    domain::IDatabaseRepository& repository_;

    std::vector<std::unique_ptr<domain::TreeNode>> roots_;
    std::unordered_map<domain::NodeId, domain::TreeNode*> index_;

    static constexpr std::uint64_t temporaryIdBase_ = 1'000'000'000ULL;
    std::uint64_t nextTemporaryId_{temporaryIdBase_};
};

} // namespace micran_tree_cache::application

#endif // MICRAN_TREE_CACHE_APPLICATION_CACHE_SERVICE_H