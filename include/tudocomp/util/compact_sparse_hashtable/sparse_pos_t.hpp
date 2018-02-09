#pragma once

#include <memory>
#include <cstdint>
#include <utility>
#include <algorithm>

#include "util.hpp"

namespace tdc {namespace compact_sparse_hashtable {

/// This type represents a position inside the compact sparse hashtable.
///
/// It is valid to have a sparse_pos_t one-past-the-end of the underlying
/// bucket vector, to act as an end-iterator.
template<typename buckets_t, typename bucket_layout_t>
class sparse_pos_t {
private:
    buckets_t* m_buckets;
public:
    using bucket_t = typename buckets_t::value_type;

    /// Index of bucket inside the hashtable
    size_t const idx_of_bucket;

    /// Bit mask of the element inside the bucket
    uint64_t const bit_mask_in_bucket;

    inline sparse_pos_t(size_t pos, buckets_t& buckets):
        m_buckets(&buckets),
        idx_of_bucket(bucket_layout_t::table_pos_to_idx_of_bucket(pos)),
        bit_mask_in_bucket(1ull << bucket_layout_t::table_pos_to_idx_inside_bucket(pos))
    {}

    /// Accesses the bucket at this sparse position.
    inline bucket_t& bucket() const {
        DCHECK_LT(idx_of_bucket, m_buckets->size());
        return (*m_buckets)[idx_of_bucket];
    }

    /// Check if the sparse position exists in the corresponding bucket.
    inline bool exists_in_bucket() const {
        // bitvector of the bucket
        uint64_t bv = bucket().bv();

        return (bv & bit_mask_in_bucket) != 0;
    }

    /// Get the idx of the element inside the corresponding bucket.
    ///
    /// It is legal to call this method even if the element at
    /// the sparse position does not exists, to calculate a position
    /// at which it should be inserted.
    inline size_t offset_in_bucket() const {
        // bitvector of the bucket
        uint64_t bv = bucket().bv();

        return __builtin_popcountll(bv & (bit_mask_in_bucket - 1));
    }
};

}}
