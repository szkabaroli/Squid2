#include <Public/Core/Random.h>

struct xorshift32_state {
    xorshift32_state(u32 a) : a(a) {}
    u32 a;
};

/* The state word must be initialized to non-zero */
u32 inline xorshift32(struct xorshift32_state *state) {
	/* Algorithm "xor" from p. 4 of Marsaglia, "Xorshift RNGs" */
	u32 x = state->a;
	x ^= x << 13;
	x ^= x >> 17;
	x ^= x << 5;
	return state->a = x;
}

struct xorshift64_state {
    xorshift64_state(uint64_t a) : a(a) {}
    uint64_t a;
};

u64 inline xorshift64(struct xorshift64_state *state) {
	u64 x = state->a;
	x ^= x << 13;
	x ^= x >> 7;
	x ^= x << 17;
	return state->a = x;
}

xorshift32_state state32 = xorshift32_state(100);
xorshift64_state state64 = xorshift64_state(100);

u32 gen_xorshift32() {
    return xorshift32(&state32);
}

u64 gen_xorshift64() {
    return xorshift64(&state64);
}