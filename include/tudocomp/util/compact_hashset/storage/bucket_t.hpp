#pragma once

#include <memory>
#include <cstdint>
#include <utility>
#include <algorithm>

#include <tudocomp/util/bit_packed_layout_t.hpp>
#include "../util.hpp"
#include "quot_ptrs_t.hpp"
#include "quot_data.hpp"

namespace tdc {namespace compact_sparse_hashset {

/// A bucket of quotient-value pairs in a sparse compact hashtable.
///
/// It consists of a pointer to a single heap allocation, that contains:
/// - A 64-bit bitvector of currently stored elements.
/// - A dynamic-width bitvector of quotients.
///
/// An empty bucket does not allocate any memory.
template<size_t N>
class bucket_t {
    std::unique_ptr<uint64_t[]> m_data;

    using qvd_t = quot_data_seq_t;
public:
    /// Maps hashtable position to position of the corresponding bucket,
    /// and the position inside of it.
    struct bucket_layout_t {
        static constexpr size_t BVS_WIDTH_SHIFT = 6;
        static constexpr size_t BVS_WIDTH_MASK = 0b111111;

        static inline size_t table_pos_to_idx_of_bucket(size_t pos) {
            return pos >> BVS_WIDTH_SHIFT;
        }

        static inline size_t table_pos_to_idx_inside_bucket(size_t pos) {
            return pos & BVS_WIDTH_MASK;
        }

        static inline size_t table_size_to_bucket_size(size_t size) {
            return (size + BVS_WIDTH_MASK) >> BVS_WIDTH_SHIFT;
        }
    };
    using quot_width_t = uint8_t;

    inline bucket_t(): m_data() {}

    /// Construct a bucket, reserving space according to the bitvector
    /// `bv` and `quot_width`.
    inline bucket_t(uint64_t bv, quot_width_t width) {
        if (bv != 0) {
            auto qvd_size = qvd_data_size(popcount(bv), width);

            m_data = std::make_unique<uint64_t[]>(qvd_size + 1);
            m_data[0] = bv;

            // NB: We call this for its alignment asserts
            ptr(width);
        } else {
            m_data.reset();
        }
    }

    inline bucket_t(bucket_t&& other) = default;
    inline bucket_t& operator=(bucket_t&& other) = default;

    /// Returns the bitvector of contained elements.
    inline uint64_t bv() const {
        if (!is_empty()) {
            return m_data[0];
        } else {
            return 0;
        }
    }

    /// Returns the amount of elements in the bucket.
    inline size_t size() const {
        return popcount(bv());
    }

    /// Returns a `quot_ptrs_t` to position `pos`,
    /// or a sentinel value that acts as a one-pass-the-end pointer.
    inline quot_ptrs_t at(size_t pos, quot_width_t width) const {
        return qvd_t::at(get_qv(), size(), pos, width);
    }

    inline bool is_allocated() const {
        return bool(m_data);
    }

    inline bool is_empty() const {
        return !bool(m_data);
    }

    inline size_t stat_allocation_size_in_bytes(quot_width_t width) const {
        if (!is_empty()) {
            return (qvd_data_size(size(), width) + 1) * sizeof(uint64_t);
        } else {
            return 0;
        }
    }

    /// Insert a new element into the bucket, growing it as needed
    inline quot_ptrs_t insert_at(
        size_t new_elem_bucket_pos,
        uint64_t new_elem_bv_bit,
        quot_width_t width)
    {
        // Just a sanity check that can not live inside or outside `bucket_t` itself.
        static_assert(sizeof(bucket_t<N>) == sizeof(void*), "unique_ptr is more than 1 ptr large!");

        // TODO: check out different sizing strategies
        // eg, the known sparse_hash repo uses overallocation for small buckets

        // create a new bucket with enough size for the new element
        auto new_bucket = bucket_t<N>(bv() | new_elem_bv_bit, width);

        auto new_iter = new_bucket.at(0, width);
        auto old_iter = at(0, width);

        auto const new_iter_midpoint = new_bucket.at(new_elem_bucket_pos, width);
        auto const new_iter_end = new_bucket.at(new_bucket.size(), width);

        quot_ptrs_t ret;

        // move all elements before the new element's location from old bucket into new bucket
        while(new_iter != new_iter_midpoint) {
            new_iter.set_quotient(old_iter.get_quotient());
            new_iter.increment_ptr();
            old_iter.increment_ptr();
        }

        // move new element into its location in the new bucket
        {
            ret = new_iter;
            new_iter.increment_ptr();
        }

        // move all elements after the new element's location from old bucket into new bucket
        while(new_iter != new_iter_end) {
            new_iter.set_quotient(old_iter.get_quotient());
            new_iter.increment_ptr();
            old_iter.increment_ptr();
        }

        // destroy old empty elements, and overwrite with new bucket
        *this = std::move(new_bucket);

        return ret;
    }
private:
    inline uint64_t* get_qv() const {
        return static_cast<uint64_t*>(m_data.get()) + 1;
    }

    inline static size_t qvd_data_size(size_t size, quot_width_t width) {
        return qvd_t::calc_sizes(size, width).overall_qword_size;
    }

    /// Creates the pointers to the beginnings of the two arrays inside
    /// the allocation.
    inline QuotPtr ptr(quot_width_t width) const {
        return qvd_t::ptr(get_qv(), size(), width);
    }
};

}}
