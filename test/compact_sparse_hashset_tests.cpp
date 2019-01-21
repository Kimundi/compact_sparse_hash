#include <gtest/gtest.h>

#include <cstdint>
#include <algorithm>

#include <tudocomp/util/compact_hashset/generic_compact_hashset.hpp>
#include <tudocomp/util/compact_hash/hash_functions.hpp>
#include <tudocomp/util/compact_hash/index_structure/cv_bvs_t.hpp>

template<typename hash_t = tdc::compact_hash::poplar_xorshift_t>
using compact_sparse_hashset_t = tdc::compact_sparse_hashset::generic_hashset_t<
    hash_t,
    tdc::compact_hash::cv_bvs_t
>;

using COMPACT_TABLE = compact_sparse_hashset_t<>;

#include "compact_hashset_tests.template.hpp"
