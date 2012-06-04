/* -*- mode: c++ -*- */
#pragma once
#include "util.hpp"
#include <stdio.h>
#include <assert.h>

struct V128 {
	__m128i x_;
	V128()
	{
	}
	V128(const uint32_t *p)
		: x_(_mm_load_si128((const __m128i*)p))
	{
	}
	V128(__m128i x)
		: x_(x)
	{
	}
	V128(__m128 x)
		: x_(_mm_castps_si128(x))
	{
	}
	__m128 to_ps() const { return _mm_castsi128_ps(x_); }
	// m = [x3:x2:x1:x0]
	V128(uint32_t x0, uint32_t x1, uint32_t x2, uint32_t x3)
		: x_(_mm_set_epi32(x3, x2, x1, x0))
	{
	}
	explicit V128(uint32_t x)
		: x_(_mm_cvtsi32_si128(x))
	{
	}
	V128(const V128& rhs)
		: x_(rhs.x_)
	{
	}
	void clear()
	{
		*this = _mm_setzero_si128();
	}
	void set(uint32_t x)
	{
		x_ = _mm_set1_epi32(x);
	}
	// m = [x3:x2:x1:x0]
	void set(uint32_t x0, uint32_t x1, uint32_t x2, uint32_t x3)
	{
		x_ = _mm_set_epi32(x3, x2, x1, x0);
	}
	// aligned
	void store(uint32_t *p) const
	{
		_mm_store_si128((__m128i*)p, x_);
	}
	// unaligned
	void store_u(uint32_t *p) const
	{
		_mm_storeu_si128((__m128i*)p, x_);
	}
	// aligned
	void load(const uint32_t *p)
	{
		x_ = _mm_load_si128((const __m128i*)p);
	}
	// unaligned
	void load_u(const uint32_t *p)
	{
		x_ = _mm_loadu_si128((const __m128i*)p);
	}
	/*
		*this >>= n
	*/
	template<int n>
	void shrBit();
	/*
		*this <<= n
	*/
	template<int n>
	void shlBit();
	void put(const char *msg = 0) const
	{
		uint32_t v[4];
		store_u(v);
		if (msg) printf("%s", msg);
		printf("%08x:%08x:%08x:%08x", v[3], v[2], v[1], v[0]);
		if (msg) putchar('\n');
	}
};

inline uint32_t movd(const V128& a)
{
	return _mm_cvtsi128_si32(a.x_);
}

inline V128 Zero()
{
	return _mm_setzero_si128();
}

template<int n>
inline V128 psrldq(const V128& a)
{
	return _mm_srli_si128(a.x_, n);
}

template<int n>
inline V128 pslldq(const V128& a)
{
	return _mm_slli_si128(a.x_, n);
}

template<int n>
inline V128 psrlq(const V128& a)
{
	return _mm_srli_epi64(a.x_, n);
}

inline V128 psrlq(const V128& a, const V128& n)
{
	return _mm_srl_epi64(a.x_, n.x_);
}

template<int n>
inline V128 psllq(const V128& a)
{
	return _mm_slli_epi64(a.x_, n);
}

inline V128 psllq(const V128& a, const V128& n)
{
	return _mm_sll_epi64(a.x_, n.x_);
}

template<int n>
inline V128 palignr(const V128& a, const V128& b)
{
	return _mm_alignr_epi8(a.x_, b.x_, n);
}

inline V128 punpckhqdq(const V128& a, const V128& b)
{
	return _mm_unpackhi_epi64(a.x_, b.x_);
}

inline V128 unpcklps(const V128& a, const V128& b)
{
	return _mm_unpacklo_ps(a.to_ps(), b.to_ps());
}

inline V128 unpckhps(const V128& a, const V128& b)
{
	return _mm_unpackhi_ps(a.to_ps(), b.to_ps());
}

inline V128 paddd(const V128& a, const V128& b)
{
	return _mm_add_epi32(a.x_, b.x_);
}

inline V128 andn(const V128& a, const V128& b)
{
	return _mm_andnot_si128(a.x_, b.x_);
}

inline V128 por(const V128& a, const V128& b)
{
	return _mm_or_si128(a.x_, b.x_);
}

inline V128 pand(const V128& a, const V128& b)
{
	return _mm_and_si128(a.x_, b.x_);
}

inline V128 pxor(const V128& a, const V128& b)
{
	return _mm_xor_si128(a.x_, b.x_);
}

inline V128 pmaxsd(const V128& a, const V128& b)
{
	return _mm_max_epi32(a.x_, b.x_);
}

inline V128 pminsd(const V128& a, const V128& b)
{
	return _mm_min_epi32(a.x_, b.x_);
}

inline V128 pmaxud(const V128& a, const V128& b)
{
	return _mm_max_epu32(a.x_, b.x_);
}

inline V128 pminud(const V128& a, const V128& b)
{
	return _mm_min_epu32(a.x_, b.x_);
}

inline V128 pcmpeqd(const V128& a, const V128& b)
{
	return _mm_cmpeq_epi32(a.x_, b.x_);
}

inline V128 pcmpgtd(const V128& a, const V128& b)
{
	return _mm_cmpgt_epi32(a.x_, b.x_);
}

inline uint32_t pmovmskb(const V128& a)
{
	return _mm_movemask_epi8(a.x_);
}

inline int ptest_zf(const V128& a, const V128& b)
{
	return _mm_testz_si128(a.x_, b.x_);
}

inline int ptest_cf(const V128& a, const V128& b)
{
	return _mm_testc_si128(a.x_, b.x_);
}
inline void swap128(uint32_t *p, uint32_t *q)
{
	V128 t(p);
	V128(q).store(p);
	t.store(q);
}

inline void copy128(uint32_t *dest, const uint32_t *src)
{
	V128(src).store(dest);
}

template<int n>
inline void V128::shrBit()
{
	assert(n < 64);
	*this = psrlq<n>(*this) | psllq<64 - n>(psrldq<8>(*this));
#if 0
	if (n == 64) {
		*this = psrldq<8>(*this);
	} else if (n > 64) {
		*this = psrlq<n - 64>(psrldq<8>(*this));
	}
#endif
}

template<int n>
inline void V128::shlBit()
{
	assert(n < 64);
	*this = psllq<n>(*this) | psrlq<64 - n>(pslldq<8>(*this));
#if 0
	if (n == 64) {
		*this = pslldq<8>(*this);
	} else if (n > 64) {
		*this = psllq<n - 64>(pslldq<8>(*this));
	}
#endif
}

/*
	byte rotr [x2:x1:x0]
*/
template<int n>
inline void rotrByte(V128& x0, V128& x1, V128& x2)
{
	V128 t(x0);
	x0 = palignr<n>(x1, x0);
	x1 = palignr<n>(x2, x1);
	x2 = palignr<n>(t, x2);
}

/*
	byte rotl [x2:x1:x0]
*/
template<int n>
inline void rotlByte(V128& x0, V128& x1, V128& x2)
{
	V128 t(x2);
	x2 = palignr<16 - n>(x2, x1);
	x1 = palignr<16 - n>(x1, x0);
	x0 = palignr<16 - n>(x0, t);
}