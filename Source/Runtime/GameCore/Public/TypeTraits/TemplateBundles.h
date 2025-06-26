// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

template <size_t v1, size_t v2>
struct ValueBundle
{
	static constexpr size_t V1 = v1;
	static constexpr size_t V2 = v2;
};