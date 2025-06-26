// Fill out your copyright notice in the Description page of Project Settings.


// *************************************************************************************************
// FixedPointSIMD is implemented entirely in free-floating functions to minimize overhead
// These functions are inline and templates, so users must follow all of 
// Google Highway's API requirements (Includes, macros, etc...)
// 
// This is done for performance and flexibility reasons. Having non-inlined operator functions 
// would be very unperformant due to the amount of function calls.
// And explicit instantiation of every <B, F> template argument is inflexible. 
// *************************************************************************************************

// Google Highway requirement
// See: https://github.com/google/highway/blob/master/hwy/examples/skeleton-inl.h
#if defined(SIMD_NUMERICS_FIXEDPOINT_SIMD_H_) == defined(HWY_TARGET_TOGGLE)
#ifdef SIMD_NUMERICS_FIXEDPOINT_SIMD_H_
#undef SIMD_NUMERICS_FIXEDPOINT_SIMD_H_
#else
#define SIMD_NUMERICS_FIXEDPOINT_SIMD_H_
#endif

// When using dynamic dispatch, foreach_target.h is included from translation units(.cc files), 
// not headers. Headers containing vector code shared between several translation units require 
// a special include guard, for example the following taken from examples / skeleton - inl.h:
#include "hwy/highway.h"
#include "OperationsSIMD.h"
#include "Numerics/FixedPoint.h"
#include "Numerics/FixedPointConstants.h"

#include "Diagnostics/DebugLog.h"
#include "Diagnostics/DebugLogSIMD.h"
// Each function that calls Highway ops (such as Load) must either be prefixed with HWY_ATTR, 
// OR reside between HWY_BEFORE_NAMESPACE() and HWY_AFTER_NAMESPACE(). 
// Lambda functions currently require HWY_ATTR before their opening brace.
HWY_BEFORE_NAMESPACE();

namespace SIMD::HWY_NAMESPACE 
{
    // *********************************************************************************************
    // Aliases
    
    // Signed types
    template <size_t B, size_t F>
    using T = FixedPoint<B, F>::base_type;

    template <size_t B, size_t F>
    using D = hn::ScalableTag<T<B, F>>;

    template <size_t B, size_t F>
    using V = hn::Vec<D<B, F>>;

    template <size_t B, size_t F>
    using M = hn::Mask<D<B, F>>;

    // Unsigned types
    template <size_t B, size_t F>
    using UT = FixedPoint<B, F>::unsigned_type;

    template <size_t B, size_t F>
    using UD = hn::ScalableTag<UT<B, F>>;

    template <size_t B, size_t F>
    using UV = hn::Vec<UD<B, F>>;

    template <size_t B, size_t F>
    using UM = hn::Mask<UD<B, F>>;

    /// Creates all permutations of 2 parameter operations with either vector or scalar inputs.
    // Scalar can be either fixed point, or arithmetic converted to fixed point.
    // Note - Do not use this with the Operations.h ops. Will cause confusion between the op
    // that converts directly from arithmetic to operation, and this one that converts it to fixed
    // point and then operation. 
#define DEFINE_FIXEDPOINT_OP_2(OpName)															\
template <size_t B, size_t F>						                                            \
HWY_INLINE V<B, F> OpName(V<B, F> a, FixedPoint<B, F> b) {									    \
    return sn::OpName<B, F>(a, sn::Broadcast<V<B, F>>(b.ToRaw()));								\
}																								\
																								\
template <size_t B, size_t F>						                                            \
HWY_INLINE V<B, F> OpName(FixedPoint<B, F> a, V<B, F> b) {										\
    return sn::OpName<B, F>(sn::Broadcast<V<B, F>>(a.ToRaw()), b);								\
}                                                                                               \
																								\
template <size_t B, size_t F, typename Number> requires std::is_arithmetic_v<Number>	        \
HWY_INLINE V<B, F> OpName(V<B, F> a, Number b) {									            \
    return sn::OpName<B, F>(a, FixedPoint<B, F>(b));						                    \
}																								\
																								\
template <size_t B, size_t F, typename Number> requires std::is_arithmetic_v<Number>	        \
HWY_INLINE V<B, F> OpName(FixedPoint<B, F> a, V<B, F> b) {										\
    return sn::OpName<B, F>(FixedPoint<B, F>(a), b);									        \
}                                                                                               \
                                                                                                \
template <size_t B, size_t F, typename N1, typename N2>                                         \
    requires std::convertible_to<N1, FixedPoint<B, F>> &&                                       \
std::convertible_to<N2, FixedPoint<B, F>>                                                       \
HWY_INLINE V<B, F> OpName(N1 a, N2 b) {                                                         \
    return sn::OpName<B, F>(FPBroadcast<B, F>(a), FPBroadcast<B, F>(b));                        \
}

// Like the above, but for 3 parameters permutation
#define DEFINE_FIXEDPOINT_OP_3(OpName)															\
template <size_t B, size_t F>			                                                        \
HWY_INLINE V<B, F> OpName(FixedPoint<B, F> x, V<B, F> a, V<B, F> b) {							\
    return sn::OpName<B, F>(sn::Broadcast<V<B, F>>(x.ToRaw()), a, b);							\
}																								\
																								\
template <size_t B, size_t F>			                                                        \
HWY_INLINE V<B, F> OpName(V<B, F> x, FixedPoint<B, F> a, V<B, F> b) {							\
    return sn::OpName<B, F>(x, sn::Broadcast<V<B, F>>(a.ToRaw()), b);							\
}																								\
template <size_t B, size_t F>			                                                        \
HWY_INLINE V<B, F> OpName(V<B, F> x, V<B, F> a, FixedPoint<B, F> b) {							\
    return sn::OpName<B, F>(x, a, sn::Broadcast<V<B, F>>(b.ToRaw()));							\
}																								\
																								\
template <size_t B, size_t F>			                                                        \
HWY_INLINE V<B, F> OpName(FixedPoint<B, F> x, FixedPoint<B, F> a, V<B, F> b) {					\
    return sn::OpName<B, F>(                                                                    \
    sn::Broadcast<V<B, F>>(x.ToRaw()), sn::Broadcast<V<B, F>>(a.ToRaw()), b                     \
    );	                                                                                        \
}																								\
																								\
template <size_t B, size_t F>			                                                        \
HWY_INLINE V<B, F> OpName(FixedPoint<B, F> x, V<B, F> a, FixedPoint<B, F> b) {					\
    return sn::OpName<B, F>(                                                                    \
    sn::Broadcast<V<B, F>>(x.ToRaw()), a, sn::Broadcast<V<B, F>>(b.ToRaw())                     \
        );	                                                                                    \
}																								\
																								\
template <size_t B, size_t F>			                                                        \
HWY_INLINE V<B, F> OpName(V<B, F> x, FixedPoint<B, F> a, FixedPoint<B, F> b) {					\
    return sn::OpName<B, F>(                                                                    \
        x, sn::Broadcast<V<B, F>>(a.ToRaw()), sn::Broadcast<V<B, F>>(b.ToRaw())                 \
    );	                                                                                        \
}																								\
																								\
template <size_t B, size_t F, typename Number> requires std::is_arithmetic_v<Number>			\
HWY_INLINE V<B, F> OpName(Number x, V<B, F> a, V<B, F> b) {										\
    return sn::OpName<B, F>(FixedPoint(x), a, b);												\
}																								\
																								\
template <size_t B, size_t F, typename Number> requires std::is_arithmetic_v<Number>			\
HWY_INLINE V<B, F> OpName(V<B, F> x, Number a, V<B, F> b) {										\
    return sn::OpName<B, F>(x, FixedPoint(a), b);												\
}																								\
																								\
template <size_t B, size_t F, typename Number> requires std::is_arithmetic_v<Number>			\
HWY_INLINE V<B, F> OpName(V<B, F> x, V<B, F> a, Number b) {										\
    return sn::OpName<B, F>(x, a, FixedPoint(b));												\
}																								\
																								\
template <size_t B, size_t F, typename Number1, typename Number2>                               \
        requires std::is_arithmetic_v<Number1> &&                                               \
	std::is_arithmetic_v<Number2>																\
HWY_INLINE V<B, F> OpName(Number1 x, Number2 a, V<B, F> b) {									\
    return sn::OpName<B, F>(FixedPoint(x), FixedPoint(a), b);									\
}																								\
																								\
template <size_t B, size_t F, typename Number1, typename Number2>                               \
        requires std::is_arithmetic_v<Number1> &&                                               \
	std::is_arithmetic_v<Number2>																\
HWY_INLINE V<B, F> OpName(V<B, F> x, Number1 a, Number2 b) {									\
    return sn::OpName<B, F>(x, FixedPoint(a), FixedPoint(b));									\
}																								\
																								\
template <size_t B, size_t F, typename Number1, typename Number2>                               \
        requires std::is_arithmetic_v<Number1> &&                                               \
	std::is_arithmetic_v<Number2>																\
HWY_INLINE V<B, F> OpName(Number1 x, V<B, F> a, Number2 b) {									\
    return sn::OpName<B, F>(FixedPoint(x), a, FixedPoint(b));									\
}

    // *********************************************************************************************
    // Initialization

    /// <summary>
    ///  Casts a numeric value into T, and broadcast it into vector<T>
    /// </summary>
    template <size_t B, size_t F>
    HWY_INLINE V<B, F> FPBroadcast(FixedPoint<B, F> t) {
        const hn::ScalableTag<hn::TFromV<V<B, F>>> d;
        return hn::Set(d, t.ToRaw());
    }

    template <size_t B, size_t F, typename Number> requires std::is_arithmetic_v<Number>
    HWY_INLINE V<B, F> FPBroadcast(Number t) {
        return sn::FPBroadcast<B, F>(FixedPoint<B, F>(t));
    }

    
    // *********************************************************************************************
    // Bitwise

    template <size_t B, size_t F>
    HWY_INLINE constexpr V<B, F> FPAnd(V<B, F> lhs, V<B, F> rhs) { return sn::And(lhs, rhs); }

    template <size_t B, size_t F>
    HWY_INLINE constexpr V<B, F> FPOr(V<B, F> lhs, V<B, F> rhs) { return sn::FPOr(lhs, rhs); }

    template <size_t B, size_t F>
    HWY_INLINE constexpr V<B, F> FPXor(V<B, F> lhs, V<B, F> rhs) { return sn::FPXor(lhs, rhs); }

    DEFINE_FIXEDPOINT_OP_2(FPAnd);
    DEFINE_FIXEDPOINT_OP_2(FPOr);
    DEFINE_FIXEDPOINT_OP_2(FPXor);

    // *********************************************************************************************
    // Arithmetics

    template <size_t B, size_t F>
    HWY_INLINE constexpr V<B, F> FPAdd(V<B, F> lhs, V<B, F> rhs) { return sn::Add(lhs, rhs); }

    template <size_t B, size_t F>
    HWY_INLINE constexpr V<B, F> FPSub(V<B, F> lhs, V<B, F> rhs) { return sn::Sub(lhs, rhs); }

    template <size_t B, size_t F>
    HWY_INLINE constexpr V<B, F> FPMul(V<B, F> lhs, V<B, F> rhs) {
        using vec = V<B, F>;

        vec a_upper = hn::ShiftRight<F>(lhs);
        vec b_upper = hn::ShiftRight<F>(rhs);
        vec a_lower = sn::And(lhs, FixedPoint<B, F>::FractionMask);
        vec b_lower = sn::And(rhs, FixedPoint<B, F>::FractionMask);

        // Equivalent to:
        // x1 = a_upper * b_upper;
        // x2 = a_upper * b_lower;
        // x3 = a_lower * b_upper;
        // x4 = a_lower * b_lower;
        // ((x1 << F) + (x2 + x3)) + ((x4 >> F) & FractionMask)
        return sn::Add(
            sn::Add(
                hn::ShiftLeft<F>(sn::Mul(a_upper, b_upper)),
                sn::Add(sn::Mul(a_upper, b_lower), sn::Mul(a_lower, b_upper))
            ),
            sn::And(
                hn::ShiftRight<F>(sn::Mul(a_lower, b_lower)),
                FixedPoint<B, F>::FractionMask
            )
        );
    }

    // Using egyptian division.
    template <size_t B, size_t F>
    HWY_INLINE constexpr V<B, F> FPDiv(V<B, F> numerator, V<B, F> denominator) {

        using vec = V<B, F>;
        using uvec = UV<B, F>;
        using mask = M<B, F>;
        using umask = UM<B, F>;
        
        if (hn::FindFirstTrue(D<B, F>(), hn::Eq(denominator, Zero<vec>())) != -1) {
            throw std::exception("division by zero");
        }

        // Division is much easier with positive numbers, so we save the sign and work on positives
        mask neg = hn::Xor(hn::IsNegative(numerator), hn::IsNegative(denominator));
        uvec num = Reinterpret<uvec>(hn::Abs(numerator));
        uvec den = Reinterpret<uvec>(hn::Abs(denominator));
        
        uvec n = num;
        uvec d = den;
        uvec x = Broadcast<uvec>(1);
        uvec ans = Zero<uvec>();

        // We want to scale the demoninator up such that it is larger than the numerator, but not
        // enough to overflow.
        // We can do this by shifting the den up towards the num, and then doing one more 
        // shift if the den is still too low. 
        //
        // Scalar equivalent:
        // 
        // while ((n >= d) && (((d >> (bits - 1)) & 1) == 0)) {
        //     x <<= 1;
        //     d <<= 1;
        // }
        vec numLeadZero = Reinterpret<vec>(hn::LeadingZeroCount(n));
        vec denLeadZero = Reinterpret<vec>(hn::LeadingZeroCount(d));
        uvec shift0 = Reinterpret<uvec>(hn::ZeroIfNegative(sn::Sub(denLeadZero, numLeadZero)));
        d = hn::Shl(d, shift0);
        x = hn::Shl(x, shift0);
        umask leMask = hn::Le(d, num);     // Shifts one more time if num still > den
        uvec shift1 = hn::IfThenElseZero(leMask, Broadcast<uvec>(1));
        d = hn::Shl(d, shift1);
        x = hn::Shl(x, shift1);

        // Scalar equivalent:
        // 
        // while (x != 0) {
        //     if (n >= d) {
        //         n -= d;
        //         answer += x;
        //     }
        //     x >>= 1;
        //     d >>= 1;
        // }
        uvec xIters = Sub(B, hn::LeadingZeroCount(x));
        int iterations = ReduceMax(xIters);

        for (int i = 0; i < iterations; i++) {
            umask xnot0 = hn::Ne(x, Zero<uvec>());

            umask nged = hn::And(hn::Ge(n, d), xnot0);
            n = hn::IfThenElse(nged, sn::Sub(n, d), n);
            ans = hn::IfThenElse(nged, sn::Add(ans, x), ans);

            uvec shift = hn::IfThenElseZero(xnot0, Broadcast<uvec>(1));
            x = hn::Shr(x, shift);
            d = hn::Shr(d, shift);
        }


        uvec l1 = n;
        uvec l2 = den;

        uvec shift2 = Reinterpret<uvec>(hn::ZeroIfNegative(sn::Sub(
            int(F), 
            Reinterpret<vec>(hn::LeadingZeroCount(l1))
        )));
        l1 = hn::Shr(l1, shift2);
        l2 = hn::Shr(l2, shift2);

        uvec lo = sn::Div(hn::ShiftLeft<F>(l1), l2);
        vec quotient = Reinterpret<vec>(Or(hn::ShiftLeft<F>(ans), lo));
        quotient = hn::IfThenElse(neg, sn::Mul(quotient, -1), quotient);
        return quotient;
    }

    DEFINE_FIXEDPOINT_OP_2(FPAdd);
    DEFINE_FIXEDPOINT_OP_2(FPSub);
    DEFINE_FIXEDPOINT_OP_2(FPMul);
    DEFINE_FIXEDPOINT_OP_2(FPDiv);

    // *********************************************************************************************
    // Extraction / Conversion

    template <size_t B, size_t F, size_t N>
    HWY_INLINE constexpr FixedPoint<B, F> FPGet(V<B, F> value) {
        return FixedPoint<B, F>::FromBase(hn::ExtractLane<T<B, F>>(value, N));
    }

    template <size_t B, size_t F>
    HWY_INLINE constexpr V<B, F> FPToInt(V<B, F> value) {
        return hn::ShiftRight<F - 1>(FPAdd<B, F>(hn::ShiftRight<1>(value), 0.25));
    }

    // *********************************************************************************************
    // Math

    // Single parameter

    template <size_t B, size_t F>
    HWY_INLINE constexpr V<B, F> FPRound(V<B, F> value) {
        return sn::And(sn::FPAdd<B, F>(value, 0.5), FixedPoint<B, F>::IntegerMask);
    }

    template <size_t B, size_t F>
    HWY_INLINE constexpr V<B, F> FPCeil(V<B, F> value) {
        return sn::FPAdd<B, F>(FPFloor<B, F>(value), 1);
    }

    template <size_t B, size_t F>
    HWY_INLINE constexpr V<B, F> FPCeil(V<B, F> value, int n) {
        return sn::FPAdd<B, F>(FPFloor<B, F>(value), FixedPoint<B, F>::FromBase((1 << F) >> n));
    }

    template <size_t B, size_t F>
    HWY_INLINE constexpr V<B, F> FPFloor(V<B, F> value) {
        return And(value, FixedPoint<B, F>::IntegerMask);
    }

    template <size_t B, size_t F>
    HWY_INLINE constexpr V<B, F> FPFloor(V<B, F> value, int n) {
        return And(value, FixedPoint<B, F>::IntegerMask >> n);
    }

    template <size_t B, size_t F>
    HWY_INLINE constexpr V<B, F> FPSquare(V<B, F> value) {
        return FPMul<B, F>(value, value);
    }

    template <size_t B, size_t F>
    HWY_INLINE constexpr V<B, F> FPSqrt(V<B, F> value, unsigned int iterations = 8) {
        using vec = V<B, F>;
        using mask = M<B, F>;

        // This mask prevents k from hitting zero. 
        // Since this is fixed point, some right bit shift may truncate k to 0, which would cause 
        // div by 0 errors. So we keep k as the smallest fp value, and set it to 0 after the
        // the algorithm is complete. 
        mask kIs0 = hn::Eq(value, Zero<vec>());             // if (value == 0) then always 0
        vec k = hn::ShiftRight<1>(FPAdd<B, F>(value, 1));

        for (unsigned int i = 0; i < iterations; ++i) {
            k = FPAdd<B, F>(k, FPDiv<B, F>(value, k));
            vec kshift = hn::ShiftRight<1>(k);

            kIs0 = Or(kIs0, hn::Eq(kshift, Zero<vec>()));
            k = hn::IfThenElse(kIs0, k, kshift);        // Keeps k not 0 if k becomes 0
        }

        return hn::IfThenElse(kIs0, Zero<vec>(), k);    // Truncates k to 0 if it was supposed to
    }

    template <size_t B, size_t F, size_t N>
    HWY_INLINE constexpr V<B, F> FPSqrt(V<B, F> value, unsigned int iterations = 8) {
        using fp = FixedPoint<B, F>;
        using fpc = FixedPointConstant<B, F>;
        using vec = V<B, F>;
        using mask = M<B, F>;

        // Initial guess:  (value + (1 << F/N)) / 2
        mask kIs0 = hn::Eq(value, Zero<vec>());
        vec k = hn::ShiftRight<1>(FPAdd<B, F>(value, 1));

        // Generalized Newton-Raphson iteration: 
        // k = (k * (N - 1) / N) + (value / k^(N - 1)) / N
        for (unsigned int i = 0; i < iterations; i++) {
            vec kPower = k;
            // Compute k^(N-1) = k * k * ... (N-1) times
            for (size_t j = 1; j < N - 1; ++j) {
                kPower = FPMul<B, F>(kPower, k);
            }

            kIs0 = hn::Eq(kPower, Zero<vec>());
            kPower = hn::IfThenElse(kIs0, Broadcast<vec>(1), kPower);
            vec valueOverKPower = sn::FPDiv<B, F>(value, kPower); // value / k^(N-1)
            k = sn::FPAdd<B, F>(
                sn::FPMul<B, F>(k, fp(N - 1) / fp(N)),
                sn::FPMul<B, F>(valueOverKPower, fpc::One / fp(N))
            );
        }

        return hn::IfThenElse(kIs0, Zero<vec>(), k);    // Truncates k to 0 if it was supposed to
    }


    // Triple parameter
    template <size_t B, size_t F>
    HWY_INLINE constexpr V<B, F> FPLerp(V<B, F> a, V<B, F> b, V<B, F> t) {
        return FPAdd<B, F>(FPMul<B, F>(FPSub<B, F>(b, a), t), a);
    }


    template <size_t B, size_t F>
    HWY_INLINE constexpr V<B, F> FPClamp(V<B, F> x, V<B, F> a, V<B, F> b) {
        return sn::Clamp(x, a, b);
    }

    DEFINE_FIXEDPOINT_OP_3(FPLerp);
    DEFINE_FIXEDPOINT_OP_3(FPClamp);

    // *********************************************************************************************
    // Trigonometry

// Finds the minimum distance point on the line between line segment AB and the point P.
    template <size_t B, size_t F>
    HWY_INLINE constexpr void FPMinimumDistancePoint(
        V<B, F> ax, V<B, F> ay, V<B, F> bx, V<B, F> by, 
        V<B, F> px, V<B, F> py, V<B, F>& outX, V<B, F>& outY
        ) {
        using vec = V<B, F>;
        using mask = M<B, F>;
        using fp = FixedPoint<B, F>;
        using fpc = FixedPointConstant<B, F>;

        vec dx = sn::Sub(bx, ax);
        vec dy = sn::Sub(by, ay);
        vec lengthSquare = FPAdd<B, F>(FPSquare<B, F>(dx),FPSquare<B, F>(dy));

        // Case where A == B
        // Since SIMD can't "early exit out", set lengthSquare to a temp value before running
        // division, then truncate that lane out at the end. This is to avoid div0 errors. 
        mask aIsb = hn::Eq(lengthSquare, Zero<vec>());
        lengthSquare = hn::IfThenElse(aIsb, FPBroadcast<B, F>(1), lengthSquare);

        // Consider the line extending the segment, parameterized as A + t (B - A).
        // We find projection of point P onto the line. 
        // It falls where t = [(P - A) dot (B-A)] / |B-A|^2
        // We clamp t from [0,1] to handle points outside the segment vw.
        vec padotba = FPAdd<B, F>(
            FPMul<B, F>(sn::Sub(px, ax), sn::Sub(bx, ax)),
            FPMul<B, F>(sn::Sub(py, ay), sn::Sub(by, ay))
        );
        vec t = sn::Clamp(FPDiv<B, F>(padotba, lengthSquare), 0, fpc::One.ToRaw());

        outX = hn::IfThenElse(aIsb, ax, sn::Add(ax, FPMul<B, F>(t, sn::Sub(bx, ax))));
        outY = hn::IfThenElse(aIsb, ay, sn::Add(ay, FPMul<B, F>(t, sn::Sub(by, ay))));
    }


}  // namespace SIMD

#undef DEFINE_FIXEDPOINT_OP_2
#undef DEFINE_FIXEDPOINT_OP_3

HWY_AFTER_NAMESPACE();


#endif  // include guard