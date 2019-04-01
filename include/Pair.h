#pragma once
#ifndef PAIR_H_398
#define PAIR_H_398

// Created by Graham Eger on 4/1/2019

// TODO: will add constructors as necessary
template<typename Type1, typename Type2>
struct Pair {
    Type1 first;
    Type2 second;

    explicit constexpr Pair(const Type1 &a, const Type2 & b) : first(a, b) {}

    explicit constexpr Pair(const Pair<Type1, Type2>& p) : first(p.first), second(p.second) {}

    // for move semantics
    constexpr Pair(Pair&&) = default;
};


#endif