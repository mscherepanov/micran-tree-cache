#ifndef MICRAN_TREE_CACHE_APPLICATION_RESET_SERVICE_H
#define MICRAN_TREE_CACHE_APPLICATION_RESET_SERVICE_H

namespace micran_tree_cache::domain {
class IDatabaseResetter;
}

namespace micran_tree_cache::application {
class CacheService;

class ResetService {
public:
    ResetService(CacheService& cache, domain::IDatabaseResetter& databaseResetter);

    void reset();

private:
    CacheService& cache_;
    domain::IDatabaseResetter& databaseResetter_;
};

} // namespace micran_tree_cache::application

#endif // MICRAN_TREE_CACHE_APPLICATION_RESET_SERVICE_H