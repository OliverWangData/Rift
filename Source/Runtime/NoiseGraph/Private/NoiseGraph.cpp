// Fill out your copyright notice in the Description page of Project Settings.

#include "NoiseGraph.h"

// Google Highway boilerplate for dynamic dispatch.
// See: https://github.com/google/highway/blob/master/hwy/examples/skeleton.cc
#undef HWY_TARGET_INCLUDE
#define HWY_TARGET_INCLUDE "Runtime\NoiseGraph\Private\NoiseGraph.cpp"
#include "hwy/foreach_target.h"

// Includes that use hwy must come after foreach_target, otherwise you get redefinition errors
#include "hwy/highway.h"

#include "Nodes/NodeBaseSIMD.h"
#include "Nodes/RandomNode.h"
#include "Nodes/PerlinNode.h"
#include "Nodes/CellularNode.h"
#include "Nodes/FractalNode.h"
#include "Nodes/WarpNode.h"
#include "Nodes/TreeNode.h"
#include "Nodes/HeightmapNode.h"

#include "Nodes/InvertNode.h"

// *************************************************************************************************
// Function parameter signature (type and argX)
#define _NG_PARAM(t, index) t arg##index
#define _NG_PARAMS_1(t1) _NG_PARAM(t1, 1)
#define _NG_PARAMS_2(t1, t2) t1 arg1, t2 arg2
#define _NG_PARAMS_3(t1, t2, t3) _NG_PARAMS_2(t1, t2), _NG_PARAM(t3, 3)
#define _NG_PARAMS_4(t1, t2, t3, t4) _NG_PARAMS_3(t1, t2, t3), _NG_PARAM(t4, 4)
#define _NG_PARAMS_5(t1, t2, t3, t4, t5) _NG_PARAMS_4(t1, t2, t3, t4), _NG_PARAM(t5, 5)
#define _NG_PARAMS(ArgCount, ...) _NG_PARAMS_##ArgCount(__VA_ARGS__)

// *************************************************************************************************
// Function arguments (argX)

#define _NG_ARGS_1 arg1
#define _NG_ARGS_2 _NG_ARGS_1, arg2
#define _NG_ARGS_3 _NG_ARGS_2, arg3
#define _NG_ARGS_4 _NG_ARGS_3, arg4
#define _NG_ARGS_5 _NG_ARGS_4, arg5
#define _NG_ARGS(ArgCount) _NG_ARGS_##ArgCount

// Downcasting from NodeBase into NodeBaseSIMD
namespace SIMD::HWY_NAMESPACE
{
	template <typename T>
	HWY_ATTR inline constexpr auto NoiseGraphMacroSIMDArgumentRules(T t) {
		if constexpr (std::is_same<T, UNoiseGraph::Sampler>::value) {
			return std::static_pointer_cast<NodeBaseSIMD<NOISEGRAPH_FP_PARAMS>>(t);
		}
		else {
			return t;
		}
	}
}

#define _NG_ARGS_SIMD_RULES(arg) NoiseGraphMacroSIMDArgumentRules(arg)
#define _NG_ARGS_SR_1 _NG_ARGS_SIMD_RULES(arg1)
#define _NG_ARGS_SR_2 _NG_ARGS_SR_1, _NG_ARGS_SIMD_RULES(arg2)
#define _NG_ARGS_SR_3 _NG_ARGS_SR_2, _NG_ARGS_SIMD_RULES(arg3)
#define _NG_ARGS_SR_4 _NG_ARGS_SR_3, _NG_ARGS_SIMD_RULES(arg4)
#define _NG_ARGS_SR_5 _NG_ARGS_SR_4, _NG_ARGS_SIMD_RULES(arg5)
#define _NG_ARGS_SR(ArgCount) _NG_ARGS_SR_##ArgCount

// Converts float into FP
// Converts FNoiseKey into Sampler
#if HWY_ONCE
template <typename T>
HWY_ATTR inline constexpr auto NoiseGraphMacroUCLASSArgumentRules(T t) {
	if constexpr (std::is_floating_point_v<T>) {
		return UNoiseGraph::Fp(t);
	}
	else if constexpr (std::is_same<T, FNoiseKey>::value) {
		return t.Get();
	}
	else {
		return t;
	}
}

#define _NG_ARGS_UCLASS_RULES(arg) NoiseGraphMacroUCLASSArgumentRules(arg)
#define _NG_ARGS_UR_1 _NG_ARGS_UCLASS_RULES(arg1)
#define _NG_ARGS_UR_2 _NG_ARGS_UR_1, _NG_ARGS_UCLASS_RULES(arg2)
#define _NG_ARGS_UR_3 _NG_ARGS_UR_2, _NG_ARGS_UCLASS_RULES(arg3)
#define _NG_ARGS_UR_4 _NG_ARGS_UR_3, _NG_ARGS_UCLASS_RULES(arg4)
#define _NG_ARGS_UR_5 _NG_ARGS_UR_4, _NG_ARGS_UCLASS_RULES(arg5)
#define _NG_ARGS_UR(ArgCount) _NG_ARGS_UR_##ArgCount
#endif

// *************************************************************************************************
// Function-Creating Macros

// Creates the free-floating functions that create a node instance 
// and boxes it into an FNoiseKey::Sampler
#define NG_CREATE_SIMD_NODE(NodeName, ArgCount, ...)											\
namespace SIMD::HWY_NAMESPACE																	\
{																								\
	static HWY_ATTR FNoiseKey::Sampler Create##NodeName##Node(									\
		_NG_PARAMS(ArgCount, __VA_ARGS__)														\
	) {																							\
		return std::make_shared<NodeName##Node<NOISEGRAPH_FP_PARAMS>>(							\
		_NG_ARGS_SR(ArgCount));																	\
	}																							\
}

// Same as above, with a single template parameter
#define NG_CREATE_SIMD_NODE_T(NodeName, TemplateType, ArgCount, ...)							\
namespace SIMD::HWY_NAMESPACE																	\
{																								\
	template <TemplateType T>																	\
	static HWY_ATTR FNoiseKey::Sampler Create##NodeName##Node(									\
		_NG_PARAMS(ArgCount, __VA_ARGS__)														\
	) {																							\
		return std::make_shared<NodeName##Node<NOISEGRAPH_FP_PARAMS, T>>(						\
		_NG_ARGS_SR(ArgCount));																	\
	}																							\
}

// Creates the free-floating functions that can dispatch at runtime into the correct simd
// node creation function. 
#if HWY_ONCE
#ifdef NG_CREATE_DISPATCH
#undef NG_CREATE_DISPATCH
#endif
#define NG_CREATE_DISPATCH(NodeName, ArgCount, ...)												\
namespace SIMD																					\
{																								\
	HWY_EXPORT(Create##NodeName##Node);															\
																								\
	static HWY_ATTR FNoiseKey::Sampler DispatchCreate##NodeName##Node(							\
		_NG_PARAMS(ArgCount, __VA_ARGS__)														\
	) {																							\
	return HWY_DYNAMIC_DISPATCH_T(Create##NodeName##Node)(_NG_ARGS(ArgCount));					\
	}																							\
}
#else
#ifndef NG_CREATE_DISPATCH
#define NG_CREATE_DISPATCH(NodeName, ArgCount, ...) 
#endif
#endif

// Similar functionality as above.
// Thing to note is that HWY_EXPORT_T creates a dispatch table based on the source code line.
// So the subsequent DYNAMIC_DISPATCH_T call needs to be on the same line. 
#if HWY_ONCE
#ifdef NG_CREATE_DISPATCH_T
#undef NG_CREATE_DISPATCH_T
#endif
#define NG_CREATE_DISPATCH_T(NodeName, TemplateType, ArgCount, ...)								\
namespace SIMD																					\
{																								\
	template <TemplateType T>																	\
	static HWY_ATTR FNoiseKey::Sampler DispatchCreate##NodeName##Node(							\
		_NG_PARAMS(ArgCount, __VA_ARGS__)														\
	) {																							\
	HWY_EXPORT_T(HWY_DISPATCH_TABLE_T(), Create##NodeName##Node<T>								\
); return HWY_DYNAMIC_DISPATCH_T(HWY_DISPATCH_TABLE_T())(_NG_ARGS(ArgCount));					\
	}																							\
}
#else
#ifndef NG_CREATE_DISPATCH_T
#define NG_CREATE_DISPATCH_T(NodeName, ArgCount, ...) 
#endif
#endif

// Creates the NoiseGraph's UFUNCTION implementation
#if HWY_ONCE
#ifdef NG_CREATE_UCLASS_IMPLEMENTATION
#undef NG_CREATE_UCLASS_IMPLEMENTATION
#endif
#define NG_CREATE_UCLASS_IMPLEMENTATION(NodeName, ArgCount, ...)								\
FNoiseKey UNoiseGraph::Get##NodeName(_NG_PARAMS(ArgCount, __VA_ARGS__)) {						\
	return FNoiseKey(SIMD::DispatchCreate##NodeName##Node(_NG_ARGS_UR(ArgCount)));				\
}
#else
#ifndef NG_CREATE_UCLASS_IMPLEMENTATION
#define NG_CREATE_UCLASS_IMPLEMENTATION(NodeName, ArgCount, ...) 
#endif
#endif


#define NG_CREATE_SIMD_DISPATCH(NodeName, ArgCount, ...)										\
NG_CREATE_SIMD_NODE(NodeName, ArgCount, __VA_ARGS__);											\
NG_CREATE_DISPATCH(NodeName, ArgCount, __VA_ARGS__);

#define NG_CREATE_SIMD_DISPATCH_T(NodeName, TemplateType, ArgCount, ...)						\
NG_CREATE_SIMD_NODE_T(NodeName, TemplateType, ArgCount, __VA_ARGS__);							\
NG_CREATE_DISPATCH_T(NodeName, TemplateType, ArgCount, __VA_ARGS__);

// *************************************************************************************************
// Node Implementation

// Requirements:
// - A shared and unique node name. (E.g. Perlin, Cellular, Invert, etc...)
// - A UFUNCTION on NoiseGraph.h with the signature "FNoiseKey Get[nodename]()"
// - A NodeBaseSIMD<B, F> inherited node "[nodename]Node<B, F>".
// - The constructor for the nodebaseSIMD node should be equivalent to the UFUNCTION, after 
//			applying the rules defined in NoiseGraphMacroUCLASSArgumentRules(T). They are:
//				- Floats turned into FixedPoint
//				- FNoiseKey turns into Sampler
// 
// 
// How this works is that each Node creation has 3 functions that need to be created for the node
// to be createable in non-simd code:
//		- Free-floating function that instantiates the SIMD node, boxed into a non-SIMD base class.
//		- Free-floating dispatch function, that can dispatch the above SIMD function from 
//			non-SIMD code.
//		- Definition of NoiseGraph's UFUNCTION for creating the node. 
// 
//  #1 must be in the HWY_NAMESPACE, which is duplicated for each intrinsic type. 
//  #2 must be in the previous namespace's sub-namespace, since that's how Highway's EXPORT_T and 
//		DYNAMIC_DISPATCH_T were implemented. 
//  #3 must be without namespaces (Since UCLASS cannot have namespaces). 
//
// In order to automate this, a macro for each has been created:
// NG_CREATE_SIMD_NODE for #1 (And templated NG_CREATE_SIMD_NODE_T)
// NG_CREATE_DISPATCH for #2 (And templated NG_CREATE_DISPATCH_T)
// NG_CREATE_UCLASS_IMPLEMENTATION for #3 (No template equivalent since template logic is needed).
//
// NG_CREATE_SIMD_NODE and NG_CREATE_DISPATCH uses the Node's constructor arguments.
// NG_CREATE_UCLASS_IMPLEMENTATION uses the UCLASS's UFUNCTION arguments (Since blueprints has type
//		limits, some types in native cpp may not be usable with blueprints).
//
// NG_CREATE_SIMD_DISPATCH and T equivalent bundles together the NG_CREATE_SIMD_NODE and 
// NG_CREATE_DISPATCH macros since they can use the same parameters. 

NG_CREATE_SIMD_DISPATCH(Random, 1, UNoiseGraph::Fp);
NG_CREATE_UCLASS_IMPLEMENTATION(Random, 1, float);

NG_CREATE_SIMD_DISPATCH(Perlin, 1, UNoiseGraph::Fp);
NG_CREATE_UCLASS_IMPLEMENTATION(Perlin, 1, float);

NG_CREATE_SIMD_NODE_T(Cellular, size_t, 2, UNoiseGraph::Fp, unsigned int);
NG_CREATE_DISPATCH_T(Cellular, size_t, 2, UNoiseGraph::Fp, unsigned int);

#if HWY_ONCE
FNoiseKey UNoiseGraph::GetCellular(int feature, float seed, int maxPointsPerGrid) {
	FNoiseKey::Sampler sampler;
	unsigned int uPoints = static_cast<unsigned int>(maxPointsPerGrid);

	if (feature == 0) {
		sampler = SIMD::DispatchCreateCellularNode<0>(Fp(seed), uPoints);
	}
	else if (feature == 1) {
		sampler = SIMD::DispatchCreateCellularNode<1>(Fp(seed), uPoints);
	}
	else if (feature == 2) {
		sampler = SIMD::DispatchCreateCellularNode<2>(Fp(seed), uPoints);
	}
	else {
		UE_LOG(
			LogTemp, Warning,
			TEXT("Cellular feature %d not implemented. Defaulting to 0."), feature
		);
		sampler = SIMD::DispatchCreateCellularNode<0>(Fp(seed), uPoints);
	}

	return FNoiseKey(sampler);
}
#endif


NG_CREATE_SIMD_DISPATCH(Fractal, 4, 
	FNoiseKey::Sampler, unsigned int, UNoiseGraph::Fp, UNoiseGraph::Fp);
NG_CREATE_UCLASS_IMPLEMENTATION(Fractal, 4, FNoiseKey, int, float, float);


NG_CREATE_SIMD_DISPATCH(Warp, 4, 
	FNoiseKey::Sampler, FNoiseKey::Sampler, unsigned int, UNoiseGraph::Fp);
NG_CREATE_UCLASS_IMPLEMENTATION(Warp, 4, FNoiseKey, FNoiseKey, int, float);

NG_CREATE_SIMD_DISPATCH(Tree, 4,
	FNoiseKey::Sampler, UNoiseGraph::Fp, unsigned int, UNoiseGraph::Fp);
NG_CREATE_UCLASS_IMPLEMENTATION(Tree, 4, FNoiseKey, float, int, float);

NG_CREATE_SIMD_DISPATCH(Heightmap, 3, FNoiseKey::Sampler, UNoiseGraph::Fp, UNoiseGraph::Fp);
NG_CREATE_UCLASS_IMPLEMENTATION(Heightmap, 3, FNoiseKey, float, float);

// Modifiers
NG_CREATE_SIMD_DISPATCH(Invert, 1, FNoiseKey::Sampler);
NG_CREATE_UCLASS_IMPLEMENTATION(Invert, 1, FNoiseKey);