// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include <variant>

template <typename T, typename Vt, bool strip_pointer = false>
struct exists_in_variant;

template <typename T, typename... Vt, bool strip_pointer>
struct exists_in_variant<T, std::variant<Vt...>, strip_pointer> : 
    std::bool_constant<(std::is_same_v<
        T, 
        std::conditional_t<strip_pointer, std::remove_pointer_t<Vt>, Vt>
    > || ...)>
{ };

template <typename T, typename Variant, bool strip_pointer = false>
constexpr bool exists_in_variant_v = exists_in_variant<T, Variant, strip_pointer>::value;