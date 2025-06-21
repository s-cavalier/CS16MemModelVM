#ifndef M__BITMAP_H__
#define M__BITMAP_H__

#include "Error.h"
#include "Array.h"

namespace ministl {

template <ministl::size_t N>
class bitset {
    using block_type = unsigned int;
    static constexpr ministl::size_t bits_per_block = 8 * sizeof(block_type);
    static constexpr ministl::size_t num_blocks =
        (N + bits_per_block - 1) / bits_per_block;

    ministl::array<block_type, num_blocks> blocks_;

    // Mask to clear unused bits in the highest block
    static constexpr block_type high_mask() {
        constexpr ministl::size_t r = N % bits_per_block;
        return r == 0
            ? ~static_cast<block_type>(0)
            : (static_cast<block_type>(1) << r) - 1;
    }

public:
    // Constructor initializes all bits to zero
    constexpr bitset() noexcept { reset(); }

    // Observers
    constexpr bool test(ministl::size_t pos) const noexcept {
        ministl::size_t idx = pos / bits_per_block;
        ministl::size_t off = pos % bits_per_block;
        return ((blocks_[idx] >> off) & static_cast<block_type>(1)) != 0;
    }

    constexpr bool operator[](ministl::size_t pos) const noexcept {
        return test(pos);
    }

    // Capacity
    static constexpr ministl::size_t size() noexcept { return N; }
    bool any() const noexcept { return count() > 0; }
    bool none() const noexcept { return !any(); }

    // Modifiers
    bitset& set(ministl::size_t pos, bool value = true) noexcept {
        ministl::size_t idx = pos / bits_per_block;
        ministl::size_t off = pos % bits_per_block;
        if (value)
            blocks_[idx] |= (static_cast<block_type>(1) << off);
        else
            blocks_[idx] &= ~(static_cast<block_type>(1) << off);
        return *this;
    }

    bitset& reset(ministl::size_t pos) noexcept {
        return set(pos, false);
    }

    bitset& reset() noexcept {
        for (ministl::size_t i = 0; i < num_blocks; ++i) {
            blocks_[i] = 0;
        }
        return *this;
    }

    bitset& flip(ministl::size_t pos) noexcept {
        ministl::size_t idx = pos / bits_per_block;
        ministl::size_t off = pos % bits_per_block;
        blocks_[idx] ^= (static_cast<block_type>(1) << off);
        return *this;
    }

    bitset& flip() noexcept {
        for (ministl::size_t i = 0; i < num_blocks; ++i) {
            blocks_[i] = ~blocks_[i];
        }
        if constexpr (num_blocks > 0) {
            blocks_[num_blocks - 1] &= high_mask();
        }
        return *this;
    }

    // Count set bits
    ministl::size_t count() const noexcept {
        ministl::size_t total = 0;
        for (ministl::size_t i = 0; i < num_blocks; ++i) {
            block_type b = blocks_[i];
            while (b) {
                b &= (b - 1);
                ++total;
            }
        }
        return total;
    }

    // Bitwise operators
    bitset& operator&=(const bitset& rhs) noexcept {
        for (ministl::size_t i = 0; i < num_blocks; ++i) {
            blocks_[i] &= rhs.blocks_[i];
        }
        return *this;
    }
    bitset& operator|=(const bitset& rhs) noexcept {
        for (ministl::size_t i = 0; i < num_blocks; ++i) {
            blocks_[i] |= rhs.blocks_[i];
        }
        return *this;
    }
    bitset& operator^=(const bitset& rhs) noexcept {
        for (ministl::size_t i = 0; i < num_blocks; ++i) {
            blocks_[i] ^= rhs.blocks_[i];
        }
        return *this;
    }

    friend constexpr bitset operator~(bitset b) noexcept {
        return b.flip();
    }
    friend constexpr bitset operator&(bitset a, const bitset& b) noexcept {
        return a &= b;
    }
    friend constexpr bitset operator|(bitset a, const bitset& b) noexcept {
        return a |= b;
    }
    friend constexpr bitset operator^(bitset a, const bitset& b) noexcept {
        return a ^= b;
    }

    // Shift operators
    friend bitset operator<<(const bitset& src, ministl::size_t pos) noexcept {
        bitset dst;
        if (pos >= N) return dst;
        ministl::size_t blk_shift = pos / bits_per_block;
        ministl::size_t bit_shift = pos % bits_per_block;
        for (ministl::size_t i = num_blocks; i-- > 0; ) {
            block_type val = 0;
            if (i >= blk_shift) {
                val = src.blocks_[i - blk_shift] << bit_shift;
                if (bit_shift != 0 && i > blk_shift) {
                    val |= src.blocks_[i - blk_shift - 1]
                           >> (bits_per_block - bit_shift);
                }
            }
            dst.blocks_[i] = val;
        }
        if constexpr (num_blocks > 0) {
            dst.blocks_[num_blocks - 1] &= high_mask();
        }
        return dst;
    }

    friend bitset operator>>(const bitset& src, ministl::size_t pos) noexcept {
        bitset dst;
        if (pos >= N) return dst;
        ministl::size_t blk_shift = pos / bits_per_block;
        ministl::size_t bit_shift = pos % bits_per_block;
        for (ministl::size_t i = 0; i < num_blocks; ++i) {
            block_type val = 0;
            if (i + blk_shift < num_blocks) {
                val = src.blocks_[i + blk_shift] >> bit_shift;
                if (bit_shift != 0 && i + blk_shift + 1 < num_blocks) {
                    val |= src.blocks_[i +blk_shift + 1]
                           << (bits_per_block - bit_shift);
                }
            }
            dst.blocks_[i] = val;
        }
        return dst;
    }

    // Comparison
    friend constexpr bool operator==(const bitset& a,
                                     const bitset& b) noexcept {
        for (ministl::size_t i = 0; i < num_blocks; ++i) {
            if (a.blocks_[i] != b.blocks_[i])
                return false;
        }
        return true;
    }
    friend constexpr bool operator!=(const bitset& a,
                                     const bitset& b) noexcept {
        return !(a == b);
    }
};

}



#endif