#ifndef MICRAN_TREE_CACHE_DOMAIN_I_DATABASE_RESETTER_H
#define MICRAN_TREE_CACHE_DOMAIN_I_DATABASE_RESETTER_H

namespace micran_tree_cache::domain {
class IDatabaseResetter {
public:
    virtual ~IDatabaseResetter() = default;

    virtual void resetToInitialState() = 0;

protected:
    IDatabaseResetter() = default;
    IDatabaseResetter(const IDatabaseResetter&) = default;
    IDatabaseResetter& operator=(const IDatabaseResetter&) = default;
    IDatabaseResetter(IDatabaseResetter&&) = default;
    IDatabaseResetter& operator=(IDatabaseResetter&&) = default;
};

} // namespace micran_tree_cache::domain

#endif // MICRAN_TREE_CACHE_DOMAIN_I_DATABASE_RESETTER_H