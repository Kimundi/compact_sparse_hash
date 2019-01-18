#pragma once

#include <tudocomp/util/compact_hashmap/storage/plain_sentinel_t.hpp>
#include <tudocomp/util/compact_hashmap/index_structure/cv_bvs_t.hpp>
#include <tudocomp/util/compact_hashmap/generic_compact_hashmap.hpp>
#include <tudocomp/util/compact_hashmap/satellite_data_config_t.hpp>
#include <tudocomp/util/compact_hash/hash_functions.hpp>

namespace tdc {namespace compact_sparse_hashmap {
using namespace compact_hash;

template<typename val_t, typename hash_t = poplar_xorshift_t>
using compact_hashmap_t = generic_hashmap_t<hash_t,
    plain_sentinel_t<satellite_data_t<val_t>>,
    cv_bvs_t
>;

}}
