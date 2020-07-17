#pragma once
#include "Types.h"
#include <functional>

// File uses code from:
// https://github.com/google/filament/blob/master/libs/utils/include/utils/Hash.h
/*
 * Copyright (C) 2018 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

inline u32 murmur3(const u32 *key, size_t wordCount, u32 seed) {
    u32 h = seed;
    size_t i = wordCount;
    do {
        u32 k = *key++;
        k *= 0xcc9e2d51u;
        k = (k << 15u) | (k >> 17u);
        k *= 0x1b873593u;
        h ^= k;
        h = (h << 13u) | (h >> 19u);
        h = (h * 5u) + 0xe6546b64u;
    } while (--i);
    h ^= wordCount;
    h ^= h >> 16u;
    h *= 0x85ebca6bu;
    h ^= h >> 13u;
    h *= 0xc2b2ae35u;
    h ^= h >> 16u;
    return h;
}

template <typename T>
struct MurmurHash {
    std::size_t operator()(const T &key) const {
        static_assert(0 == (sizeof(key) & 3u), "Hashing requires a size that is a multiple of 4.");
        return murmur3((const u32 *)&key, sizeof(key) / 4, 0);
    }
};

template <class T>
inline void combine(size_t &seed, const T &v) {
    std::hash<T> hasher;
    seed ^= hasher(v) + 0x9e3779b9u + (seed << 6u) + (seed >> 2u);
}

template <class T>
inline void combine_fast(size_t &seed, const T &v) {
    std::hash<T> hasher;
    seed ^= hasher(v) << 1u;
}