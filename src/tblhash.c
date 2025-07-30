#include <cu/tblhash.h>
#include <cu/rand.h>
#include <assert.h>
#include <limits.h>
#include <string.h>

const uint64_t SIPHASH_CONSTS[4] = {
	UINT64_C(0x736f6d6570736575),
	UINT64_C(0x646f72616e646f6d),
	UINT64_C(0x6c7967656e657261),
	UINT64_C(0x7465646279746573),
};

// SipHash 1-3 is default
#ifndef SIPHASH_C
#define SIPHASH_C 1
#endif

#ifndef SIPHASH_D
#define SIPHASH_D 3
#endif


static_assert(sizeof(uint64_t) == 8, "uint64_t is not 8 bytes");
static_assert(CHAR_BIT == 8, "CHAR_BIT is not 8 bits");

int cu_tblhash_init(cu_tblhash_key *key)
{
	return cu_rand_bytes((uint8_t *)key, CU_TBLHASH_KEYSIZE);
}

void cu_tblhash_init_from_bytes(cu_tblhash_key *key, uint8_t *bytes)
{
	memcpy(key, bytes, CU_TBLHASH_KEYSIZE);
}

static void get_state(
	uint64_t *state,
	const cu_tblhash_key *key
) {
	state[0] = SIPHASH_CONSTS[0] ^ key->key[0];
	state[1] = SIPHASH_CONSTS[1] ^ key->key[1];
	state[2] = SIPHASH_CONSTS[2] ^ key->key[0];
	state[3] = SIPHASH_CONSTS[3] ^ key->key[1];
}

static inline uint64_t parse_little_endian(const uint8_t *buf)
{
	uint64_t outval = 0;
	outval |= (uint64_t)buf[0];
	outval |= (uint64_t)buf[1] << 8;
	outval |= (uint64_t)buf[2] << 16;
	outval |= (uint64_t)buf[3] << 24;
	outval |= (uint64_t)buf[4] << 32;
	outval |= (uint64_t)buf[5] << 40;
	outval |= (uint64_t)buf[6] << 48;
	outval |= (uint64_t)buf[7] << 56;
	return outval;
}

static inline uint64_t rotl(uint64_t val, unsigned amount)
{
	return (val << amount) | (val >> (64 - amount));
}

static inline void SipRound(uint64_t *state)
{
        state[0] += state[1];
        state[1] = rotl(state[1], 13);
        state[1] ^= state[0];
        state[0] = rotl(state[0], 32);

        state[2] += state[3];
        state[3] = rotl(state[3], 16);
        state[3] ^= state[2];

        state[0] += state[3];
        state[3] = rotl(state[3], 21);
        state[3] ^= state[0];

        state[2] += state[1];
        state[1] = rotl(state[1], 17);
        state[1] ^= state[2];
        state[2] = rotl(state[2], 32);
}

static inline void process_msg_word(uint64_t *state, uint64_t word) {
	state[3] ^= word; 
	for (size_t j = 0; j < SIPHASH_C; ++j) {
		SipRound(state);
	}
	state[0] ^= word;
}

static inline uint64_t pad_u64(const uint8_t *buf, size_t nbytes)
{
	assert(nbytes < 8);
	uint8_t tmp_buf[8] = {0};
	memcpy(tmp_buf, buf, nbytes);
	return parse_little_endian(tmp_buf);
}

static inline uint64_t get_last(const uint8_t *buf, size_t len)
{
	size_t lastind = (len / 8) * 8;
	uint64_t val = pad_u64(buf + lastind, len - lastind);
	val |= ((uint64_t)len & 0xFF) << 56;
	return val;
}

uint64_t cu_tblhash_hash(
	const cu_tblhash_key *key,
	const uint8_t *buf,
	size_t size
) {
	uint64_t state[4];
	get_state(state, key);
	for (size_t i = 0; i < size / 8; ++i) {
		process_msg_word(state, parse_little_endian(buf + i * 8));
	}
	process_msg_word(state, get_last(buf, size));
	state[2] ^= 0xFF;
	for (size_t i = 0; i < SIPHASH_D; ++i) {
		SipRound(state);
	}
	return state[0] ^ state[1] ^ state[2] ^ state[3];
}

