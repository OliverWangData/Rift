// Fill out your copyright notice in the Description page of Project Settings.


#include "Dev/TestActor.h"
#include "NoiseGraph.h"
#include "Diagnostics/DebugLog.h"

#include "Numerics/FixedPointConstants.h"

// Sets default values
ATestActor::ATestActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if 
	// you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void ATestActor::BeginPlay()
{
	Super::BeginPlay();

	using TOut = uint32_t;
	using fp = FixedPoint<32, 16>;
	using fpc = FixedPointConstant<32, 16>;
	
	UNoiseGraph::SamplingParameters params;
	params.Spacing = 1 / fp(2);
	params.Add(-5, 64);
	params.Add(-5, 64);
	
	UNoiseGraph::AlignedArray<TOut> aligned = UNoiseGraph::Allocate<TOut>(params);
	UNoiseGraph* noise = NewObject<UNoiseGraph>();

	FNoiseKey random = UNoiseGraph::GetRandom();
	FNoiseKey tree = UNoiseGraph::GetTree(random, 0, 0, 0.3);
	noise->Output = tree;

	noise->Sample(params, aligned);
	Log(UNoiseGraph::Fp::FromBase(aligned.GetPtr()[0]));
	Log(UNoiseGraph::Fp::FromBase(aligned.GetPtr()[1]));
	Log(UNoiseGraph::Fp::FromBase(aligned.GetPtr()[2]));
	Log(UNoiseGraph::Fp::FromBase(aligned.GetPtr()[3]));
	Log(UNoiseGraph::Fp::FromBase(aligned.GetPtr()[4]));
	Log(UNoiseGraph::Fp::FromBase(aligned.GetPtr()[5]));
	Log(UNoiseGraph::Fp::FromBase(aligned.GetPtr()[6]));
	Log(UNoiseGraph::Fp::FromBase(aligned.GetPtr()[7]));
}

// Called every frame
void ATestActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

