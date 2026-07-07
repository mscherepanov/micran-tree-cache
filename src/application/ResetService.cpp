#include "application/ResetService.h"

#include "application/CacheService.h"
#include "domain/IDatabaseResetter.h"

namespace micran_tree_cache::application {

ResetService::ResetService(CacheService& cache, domain::IDatabaseResetter& databaseResetter)
    : cache_{cache}, databaseResetter_{databaseResetter} {}

void ResetService::reset() {
    cache_.clear();
    databaseResetter_.resetToInitialState();
}

} // namespace micran_tree_cache::application