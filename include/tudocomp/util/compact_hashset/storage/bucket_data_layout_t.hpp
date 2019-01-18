
#pragma once

#include <memory>
#include <cstdint>
#include <utility>
#include <algorithm>

#include <tudocomp/util/bit_packed_layout_t.hpp>
#include <tudocomp/util/compact_hash/util.hpp>
#include "entry_ptr_t.hpp"

namespace tdc {namespace compact_sparse_hashset {
using namespace compact_hash;

struct quot_data_seq_t {
    /// Calculates the offsets of the two different arrays inside the allocation.
    struct Layout {
        cbp::cbp_layout_element_t<dynamic_t> quots_layout;
        size_t overall_qword_size;

        inline Layout(): quots_layout(), overall_qword_size(0) {
        }
    };
    inline static Layout calc_sizes(size_t size, uint8_t quot_width) {
        DCHECK_NE(size, 0);

        auto layout = cbp::bit_layout_t();

        // The quotients
        auto quots = layout.cbp_elements<dynamic_t>(size, quot_width);

        Layout r;
        r.quots_layout = quots;
        r.overall_qword_size = layout.get_size_in_uint64_t_units();
        return r;
    }

    /// Creates the pointers to the beginnings of the two arrays inside
    /// the allocation.
    inline static quot_ptrs_t ptr(uint64_t* alloc, size_t size, uint8_t quot_width) {
        DCHECK(size != 0);
        auto layout = calc_sizes(size, quot_width);

        return layout.quots_layout.ptr_relative_to(alloc);
    }

    // Run destructors of each element in the bucket.
    inline static void destroy_vals(uint64_t*, size_t, uint8_t) {
        // NB: this does not contain values
    }

    /// Returns a `val_quot_ptrs_t` to position `pos`,
    /// or a sentinel value that acts as a one-pass-the-end pointer for the empty case.
    inline static quot_ptrs_t at(uint64_t* alloc, size_t size, size_t pos, uint8_t quot_width) {
        if(size != 0) {
            auto ps = ptr(alloc, size, quot_width);
            return quot_ptrs_t(ps.quot_ptr() + pos);
        } else {
            DCHECK_EQ(pos, 0);
            return quot_ptrs_t();
        }
    }

    inline static void move_to_ptr_from_ptr(quot_ptrs_t to, quot_ptrs_t from) {
        to.set_quotient(from.get_quotient());
    }

    inline static bool compare_eq(quot_ptrs_t lhs, quot_ptrs_t rhs) {
        return lhs.get_quotient() == rhs.get_quotient();
    }
};

}}