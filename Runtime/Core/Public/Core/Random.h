#pragma once
#include "Types.h"

u32 gen_xorshift32();
u64 gen_xorshift64();

#define RANDOM_32 gen_xorshift32()
#define RANDOM_64 gen_xorshift64()