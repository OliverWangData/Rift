// Fill out your copyright notice in the Description page of Project Settings.

// Google Highway requirement
#if defined(SIMD_OPERATIONS_SIMD_H_) == defined(HWY_TARGET_TOGGLE)
#ifdef SIMD_OPERATIONS_SIMD_H_
#undef SIMD_OPERATIONS_SIMD_H_
#else
#define SIMD_OPERATIONS_SIMD_H_
#endif

#include "hwy/highway.h"

HWY_BEFORE_NAMESPACE();
namespace SIMD::HWY_NAMESPACE
{
	namespace hn = hwy::HWY_NAMESPACE;
	namespace sn = SIMD::HWY_NAMESPACE;

	/// Creates all permutations of 2 parameter operations with either vector or scalar inputs.
#define DEFINE_SCALAR_OP_2(OpName)																\
template <class V, typename Number> requires std::is_arithmetic_v<Number>						\
HWY_INLINE V OpName(V a, Number b) {															\
    return sn::OpName(a, sn::Broadcast<V>(b));													\
}																								\
																								\
template <class V, typename Number> requires std::is_arithmetic_v<Number>						\
HWY_INLINE V OpName(Number a, V b) {															\
    return sn::OpName(sn::Broadcast<V>(a), b);													\
}

	/// Creates all permutations of 3 parameter operations with either vector or scalar inputs.
#define DEFINE_SCALAR_OP_3(OpName)																\
template <class V, typename Number> requires std::is_arithmetic_v<Number>						\
HWY_INLINE V OpName(Number x, V a, V b) {														\
    return sn::OpName(sn::Broadcast<V>(x), a, b);												\
}																								\
																								\
template <class V, typename Number> requires std::is_arithmetic_v<Number>						\
HWY_INLINE V OpName(V x, Number a, V b) {														\
    return sn::OpName(x, sn::Broadcast<V>(a), b);												\
}																								\
																								\
template <class V, typename Number> requires std::is_arithmetic_v<Number>						\
HWY_INLINE V OpName(V x, V a, Number b) {														\
    return sn::OpName(x, a, sn::Broadcast<V>(b));												\
}																								\
																								\
template <class V, typename Number1, typename Number2>											\
		requires std::is_arithmetic_v<Number1> &&												\
	std::is_arithmetic_v<Number2>																\
HWY_INLINE V OpName(Number1 x, Number2 a, V b) {												\
    return sn::OpName(sn::Broadcast<V>(x), sn::Broadcast<V>(a), b);								\
}																								\
																								\
template <class V, typename Number1, typename Number2>											\
		requires std::is_arithmetic_v<Number1> &&												\
	std::is_arithmetic_v<Number2>																\
HWY_INLINE V OpName(V x, Number1 a, Number2 b) {												\
    return sn::OpName(x, sn::Broadcast<V>(a), sn::Broadcast<V>(b));								\
}																								\
																								\
template <class V, typename Number1, typename Number2>											\
		requires std::is_arithmetic_v<Number1> &&												\
	std::is_arithmetic_v<Number2>																\
HWY_INLINE V OpName(Number1 x, V a, Number2 b) {												\
    return sn::OpName(sn::Broadcast<V>(x), a, sn::Broadcast<V>(b));								\
}																								\
																								\
template <class V, typename Number1, typename Number2, typename Number3>						\
		requires std::is_arithmetic_v<Number1> &&												\
	std::is_arithmetic_v<Number2> &&															\
	std::is_arithmetic_v<Number3>																\
HWY_INLINE V OpName(Number1 x, Number2 a, Number3 b) {											\
    return sn::OpName(sn::Broadcast<V>(x), sn::Broadcast<V>(a), sn::Broadcast<V>(b));			\
}

	// *********************************************************************************************
	// Initialization
	template <class V>
	HWY_INLINE V Zero() {
		const hn::ScalableTag<hn::TFromV<V>> d;
		return hn::Zero(d);
	}

	/// <summary>
	///  Casts a numeric value into T, and broadcast it into vector<T>
	/// </summary>
	template <class V, typename Number> requires std::is_arithmetic_v<Number>
	HWY_INLINE V Broadcast(Number t) {
		const hn::ScalableTag<hn::TFromV<V>> d;
		return hn::Set(d, static_cast<hn::TFromV<V>>(t));
	}

	template <class V>
	HWY_INLINE hn::TFromV<V> Get(V v, size_t i) {
		return hn::ExtractLane(v, i);
	}

	// *********************************************************************************************
	// Demote/Promote with Memory
	template <typename TIn, typename TOut>
	HWY_INLINE void Store(hn::Vec<hn::ScalableTag<TIn>> val, TOut* HWY_RESTRICT array) {
		using id = hn::ScalableTag<TIn>;
		using ivec = hn::Vec<id>;

		// Needs to demote TIn to TOut
		if constexpr (sizeof(TOut) < sizeof(TIn)) {
			using od = hn::Rebind<TOut, id>;
			using ovec = hn::Vec<od>;

			ovec outval = hn::DemoteTo(od(), val);
			hn::StoreN(outval, od(), array, hn::Lanes(id()));
		}
		// No promote needed
		// Current implementation is to just use the bits rather than make destructive changes
		else if constexpr (sizeof(TOut) == sizeof(TIn)) {
			using od = hn::ScalableTag<TOut>;
			using ovec = hn::Vec<od>;

			ovec outval = Reinterpret<ovec>(val);
			hn::Store(outval, od(), array);
		}
		// Needs to promote TIn to TOut
		else {

		}
	}

	// *********************************************************************************************
	// Bitwise

	template <class V>
	HWY_INLINE V And(V a, V b) { return hn::And(a, b); }

	template <class V>
	HWY_INLINE V Or(V a, V b) { return hn::Or(a, b); }

	template <class V>
	HWY_INLINE V Xor(V a, V b) { return hn::Xor(a, b); }

	DEFINE_SCALAR_OP_2(And);
	DEFINE_SCALAR_OP_2(Or);
	DEFINE_SCALAR_OP_2(Xor);

	// *********************************************************************************************
	// Arithmetics

	template <class V>
	HWY_INLINE V Add(V a, V b) { return hn::Add(a, b); }
	
	template <class V>
	HWY_INLINE V Sub(V a, V b) { return hn::Sub(a, b); }

	template <class V>
	HWY_INLINE V Mul(V a, V b) { return hn::Mul(a, b); }

	template <class V>
	HWY_INLINE V Div(V a, V b) { return hn::Div(a, b); }

	template <class V>
	HWY_INLINE V Mod(V a, V b) { return hn::Mod(a, b); }

	DEFINE_SCALAR_OP_2(Add);
	DEFINE_SCALAR_OP_2(Sub);
	DEFINE_SCALAR_OP_2(Mul);
	DEFINE_SCALAR_OP_2(Div);
	DEFINE_SCALAR_OP_2(Mod);

	// *********************************************************************************************
	// Conversions

	/// <summary>
	/// Converts from one V type to another. 
	/// </summary>
	template <class TargetV, class SourceV>
	HWY_INLINE TargetV Reinterpret(SourceV a) {
		const hn::DFromV<TargetV> d;
		return hn::BitCast(d, a);
	}

	// *********************************************************************************************
	// Comparisons
	template <class V>
	HWY_INLINE V Max(V a, V b) { return hn::Max(a, b); }

	template <class V>
	HWY_INLINE V Min(V a, V b) { return hn::Min(a, b); }

	DEFINE_SCALAR_OP_2(Max);
	DEFINE_SCALAR_OP_2(Min);

	
	// *********************************************************************************************
	// Reductions

	/// <summary>
	/// Get the max value of all elements in the vector. 
	/// </summary>
	template <class V>
	HWY_INLINE hn::TFromV<V> ReduceMax(V a) {
		const hn::DFromV<V> d;
		return hn::ReduceMax(d, a);
	}



	// *********************************************************************************************
	// Math
	template <class V>
	HWY_INLINE V Square(V a) {
		return sn::Mul(a, a);
	}

	template <class V>
	HWY_INLINE V Clamp(V x, V a, V b) {
		return hn::Clamp(x, a, b);
	}

	DEFINE_SCALAR_OP_3(Clamp);


}

#undef DEFINE_SCALAR_OP
#undef DEFINE_SCALAR_OP_3

HWY_AFTER_NAMESPACE();

#endif  // include guard