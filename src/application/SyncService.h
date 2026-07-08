#ifndef MICRAN_TREE_CACHE_APPLICATION_SYNC_SERVICE_H
#define MICRAN_TREE_CACHE_APPLICATION_SYNC_SERVICE_H

#include "domain/DatabaseTypes.h"

namespace micran_tree_cache::domain {
class IDatabaseRepository;
class TreeNode;
} // namespace micran_tree_cache::domain

namespace micran_tree_cache::application {
class CacheService;

class SyncService {
public:
    struct Result {
        int created{0};
        int modified{0};
        int deleted{0};

        [[nodiscard]] int total() const noexcept { return created + modified + deleted; }
    };

    SyncService(CacheService& cache, domain::IDatabaseRepository& repository);

    Result apply();

private:
    void collectChanges(const domain::TreeNode* node, domain::ChangeSet& changes,
                        Result& summary) const;
    void reconcileCache(const domain::ApplyResult& applyResult);
    void markSubtreeSynced(domain::TreeNode* node);

    CacheService& cache_;
    domain::IDatabaseRepository& repository_;
};

} // namespace micran_tree_cache::application

#endif // MICRAN_TREE_CACHE_APPLICATION_SYNC_SERVICE_H