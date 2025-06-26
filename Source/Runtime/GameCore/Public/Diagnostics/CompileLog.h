// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

// Hack for outputting type name as a compiler error.
// Usage: CompileLogType<SomeType>{};
// See: 
// https://stackoverflow.com/questions/44799905/how-to-output-c-type-information-during-compilation
template <typename...> struct CompileLogType;
class CompileLogUnusedStruct {};