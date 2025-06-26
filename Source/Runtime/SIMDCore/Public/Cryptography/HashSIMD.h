// Fill out your copyright notice in the Description page of Project Settings.

// Google Highway requirement
#if defined(SIMD_CRYPTOGRAPHY_HASH_SIMD_H_) == defined(HWY_TARGET_TOGGLE)
#ifdef SIMD_CRYPTOGRAPHY_HASH_SIMD_H_
#undef SIMD_CRYPTOGRAPHY_HASH_SIMD_H_
#else
#define SIMD_CRYPTOGRAPHY_HASH_SIMD_H_
#endif

#include "hwy/highway.h"
#include "OperationsSIMD.h"

HWY_BEFORE_NAMESPACE();
namespace SIMD::HWY_NAMESPACE
{
    // Unsigned bit scrambling

    // Equivalent to: 
    // h = (h ^ (h >> 31)) * 0x85EBCA77C2B2AE63UL;
    // h = (h ^ (h >> 29)) * 0x94D049BB133111EBUL;
    // return (h ^ (h >> 30)) * 0x7D5A9B6F1550D39F;
    template <class V>
    HWY_INLINE constexpr V Scramble(V h) {
        h = Mul(Xor(h, hn::ShiftRight<31>(h)), 0x85EBCA77C2B2AE63UL);
        h = Mul(Xor(h, hn::ShiftRight<29>(h)), 0x94D049BB133111EBUL);
        return Mul(Xor(h, hn::ShiftRight<30>(h)), 0x7D5A9B6F1550D39F);
    }

    template <class V> requires std::is_unsigned_v<hn::TFromV<V>>
    HWY_INLINE constexpr V Hash(V a) {
        // Equivalent to: 
        // h = a * 0x5D588B656C078965
        V h = Mul(a, 0x5D588B656C078965);
        return Scramble(h);
    }

    template <class V> requires std::is_unsigned_v<hn::TFromV<V>>
    HWY_INLINE constexpr V Hash(V a, V b) {
        // Equivalent to: 
        // h = a * 0x517CC1B727220A95 + b * 0x54D2B4FC190DCD52
        V h = hn::MulAdd(a, Broadcast<V>(0x517CC1B727220A95), Mul(b, 0x54D2B4FC190DCD52));
        return Scramble(h);
    }

    template <class V> requires std::is_unsigned_v<hn::TFromV<V>>
    HWY_INLINE constexpr V Hash(V a, V b, V c) {
        // Equivalent to: 
        // h = a * 0x5D588B656C078965 + b * 0x6C5A13E6B79C54C3 + c * 0x7D5A9B6F1550D39F
        V h = hn::MulAdd(a, Broadcast<V>(0x5D588B656C078965),
            hn::MulAdd(b, Broadcast<V>(0x6C5A13E6B79C54C3),
                Mul(c, 0x7D5A9B6F1550D39F)
        ));
        return Scramble(h);
    }

    template <class V> requires std::is_unsigned_v<hn::TFromV<V>>
    HWY_INLINE constexpr V Hash(V a, V b, V c, V d) {
        // Equivalent to: 
        // h = a * 0x7F4A7C15F8D5C67B + b * 0x6D1CE4E5B9BF5847 + 
        // c * 0x5D588B656C078965 + d * 0x9E3779B97F4A7C15
        V h = hn::MulAdd(a, Broadcast<V>(0x7F4A7C15F8D5C67B),
            hn::MulAdd(b, Broadcast<V>(0x6D1CE4E5B9BF5847),
                hn::MulAdd(c, Broadcast<V>(0x5D588B656C078965),
                    Mul(d, 0x9E3779B97F4A7C15)
        )));
        return Scramble(h);
    }

    template <class V> requires std::is_unsigned_v<hn::TFromV<V>>
    HWY_INLINE constexpr V Hash(V a, V b, V c, V d, V e) {
        // Equivalent to: 
        // h = a * 0x8D2D7D6B7F3B2F81 + b * 0x9E3779B97F4A7C15 + c * 0x5D588B656C078965 + 
        // d * 0x6C5A13E6B79C54C3 + e * 0x7F4A7C15F8D5C67B
        V h = hn::MulAdd(a, Broadcast<V>(0x8D2D7D6B7F3B2F81),
            hn::MulAdd(b, Broadcast<V>(0x9E3779B97F4A7C15),
                hn::MulAdd(c, Broadcast<V>(0x5D588B656C078965),
                    hn::MulAdd(d, Broadcast<V>(0x6C5A13E6B79C54C3),
                        Mul(e, 0x7F4A7C15F8D5C67B)
        ))));
        return Scramble(h);
    }
}
HWY_AFTER_NAMESPACE();

#endif  // include guard