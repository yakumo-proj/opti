#include <stdio.h>
#include <memory.h>
#include <cybozu/benchmark.hpp>
#include <cybozu/xorshift.hpp>
#include <string>
#ifdef _WIN32
#include <intrin.h>
#else
#include <x86intrin.h>
#endif

static inline uint64_t local_get1(const char *p)
{
  return (uint8_t)*p;
}
static inline uint64_t local_get2(const char *p)
{
  uint16_t r;
  memcpy(&r, p, 2);
  return r;
}
static inline uint64_t local_get3(const char *p)
{
  return local_get2(p) | (local_get1(p + 2) << 16);
}
static inline uint64_t local_get4(const char *p)
{
  uint32_t r;
  memcpy(&r, p, 4);
  return r;
}
static inline uint64_t local_get3f(const char *p)
{
  if (((size_t)p && 0xfff) == 0xffd) {
    return local_get2(p) | (local_get1(p + 2) << 16);
  } else {
    return local_get4(p);
  }
}
static inline uint64_t local_get5(const char *p)
{
  return local_get4(p) | (local_get1(p + 4) << 32);
}
static inline uint64_t local_get6(const char *p)
{
  return local_get4(p) | (local_get2(p + 4) << 32);
}
static inline uint64_t local_get7(const char *p)
{
  return local_get4(p) | (local_get3(p + 4) << 32);
}
static inline uint64_t local_get8(const char *p)
{
  uint64_t r;
  memcpy(&r, p, 8);
  return r;
}
static inline uint64_t local_cmp1(const char *p, const char *q) { return local_get1(p) ^ local_get1(q); }
static inline uint64_t local_cmp2(const char *p, const char *q) { return local_get2(p) ^ local_get2(q); }
static inline uint64_t local_cmp3(const char *p, const char *q) { return local_get3(p) ^ local_get3(q); }
static inline uint64_t local_cmp4(const char *p, const char *q) { return local_get4(p) ^ local_get4(q); }
static inline uint64_t local_cmp5(const char *p, const char *q) { return local_get5(p) ^ local_get5(q); }
static inline uint64_t local_cmp6(const char *p, const char *q) { return local_get6(p) ^ local_get6(q); }
static inline uint64_t local_cmp7(const char *p, const char *q) { return local_get7(p) ^ local_get7(q); }
static inline uint64_t local_cmp8(const char *p, const char *q) { return local_get8(p) ^ local_get8(q); }
static inline uint64_t local_cmp9(const char *p, const char *q)  { return local_cmp8(p, q) | local_cmp1(p + 8, q + 8); }
static inline uint64_t local_cmp10(const char *p, const char *q) { return local_cmp8(p, q) | local_cmp2(p + 8, q + 8); }
static inline uint64_t local_cmp11(const char *p, const char *q) { return local_cmp8(p, q) | local_cmp3(p + 8, q + 8); }
static inline uint64_t local_cmp12(const char *p, const char *q) { return local_cmp8(p, q) | local_cmp4(p + 8, q + 8); }
static inline uint64_t local_cmp13(const char *p, const char *q) { return local_cmp8(p, q) | local_cmp5(p + 8, q + 8); }
static inline uint64_t local_cmp14(const char *p, const char *q) { return local_cmp8(p, q) | local_cmp6(p + 8, q + 8); }
static inline uint64_t local_cmp15(const char *p, const char *q) { return local_cmp8(p, q) | local_cmp7(p + 8, q + 8); }
static inline uint64_t local_cmp16(const char *p, const char *q)
{
#if 0
  return local_cmp8(p, q) | local_cmp8(p + 8, q + 8);
#else
  __m128i x = _mm_loadu_si128((const __m128i*)p);
  __m128i y = _mm_loadu_si128((const __m128i*)q);
#if 0
  return _mm_movemask_epi8(_mm_cmpeq_epi8(x, y)) == 0xffff;
#else
  return _mm_testc_si128(x, y) == 0;
#endif
#endif
}
static inline uint64_t local_cmp17(const char *p, const char *q) { return local_cmp16(p, q) || local_cmp1(p + 16, q + 16); }
static inline uint64_t local_cmp18(const char *p, const char *q) { return local_cmp16(p, q) || local_cmp2(p + 16, q + 16); }
static inline uint64_t local_cmp19(const char *p, const char *q) { return local_cmp16(p, q) || local_cmp3(p + 16, q + 16); }
static inline uint64_t local_cmp20(const char *p, const char *q) { return local_cmp16(p, q) || local_cmp4(p + 16, q + 16); }
static inline uint64_t local_cmp21(const char *p, const char *q) { return local_cmp16(p, q) || local_cmp5(p + 16, q + 16); }
static inline uint64_t local_cmp22(const char *p, const char *q) { return local_cmp16(p, q) || local_cmp6(p + 16, q + 16); }
static inline uint64_t local_cmp23(const char *p, const char *q) { return local_cmp16(p, q) || local_cmp7(p + 16, q + 16); }
static inline uint64_t local_cmp24(const char *p, const char *q) { return local_cmp16(p, q) || local_cmp8(p + 16, q + 16); }
static inline uint64_t local_cmp25(const char *p, const char *q) { return local_cmp16(p, q) || local_cmp9(p + 16, q + 16); }
static inline uint64_t local_cmp26(const char *p, const char *q) { return local_cmp16(p, q) || local_cmp10(p + 16, q + 16); }

static inline __m128i load16sub(const void *p, size_t shift)
{
  static const unsigned char shiftPtn[32] = {
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
    0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80
  };
  __m128i v = _mm_loadu_si128((const __m128i*)p);
  return _mm_shuffle_epi8(v, *(const __m128i*)(shiftPtn + shift));
}

static inline __m128i load16(const void *p, size_t n)
{
#if 1 // load data near page boundary safely
  const size_t addr = (size_t)p;
  const size_t bound = addr & 0xfff;
  if (bound >= 0xff1 && bound <= 0xfff - n) {
    return load16sub((const char*)(addr & -16), addr & 0xf);
  }
#endif
  return _mm_loadu_si128((const __m128i*)p);
}

static inline uint64_t mask(int x) { return (uint64_t(1) << x) - 1; }

static inline uint64_t local_cmp2u(const char *p, const char *q) { return (local_get2(p) ^ local_get4(q)) & mask(16); }
static inline uint64_t local_cmp3u(const char *p, const char *q) { return (local_get3(p) ^ local_get4(q)) & mask(24); }
static inline uint64_t local_cmp5u(const char *p, const char *q) { return (local_get5(p) ^ local_get8(q)) & mask(40); }
static inline uint64_t local_cmp6u(const char *p, const char *q) { return (local_get6(p) ^ local_get8(q)) & mask(48); }
static inline uint64_t local_cmp7u(const char *p, const char *q) { return (local_get7(p) ^ local_get8(q)) & mask(56); }

static inline uint64_t local_cmp10u(const char *p, const char *q) { return local_cmp8(p, q) | local_cmp2u(p + 8, q + 8); }
static inline uint64_t local_cmp11u(const char *p, const char *q) { return local_cmp8(p, q) | local_cmp3u(p + 8, q + 8); }
static inline uint64_t local_cmp12u(const char *p, const char *q) { return local_cmp8(p, q) | local_cmp4(p + 8, q + 8); }
static inline uint64_t local_cmp13u(const char *p, const char *q) { return local_cmp8(p, q) | local_cmp5u(p + 8, q + 8); }
static inline uint64_t local_cmp14u(const char *p, const char *q) { return local_cmp8(p, q) | local_cmp6u(p + 8, q + 8); }
static inline uint64_t local_cmp15u(const char *p, const char *q) { return local_cmp8(p, q) | local_cmp7u(p + 8, q + 8); }

const uint8_t maskTbl[32] = {
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
};
/*
	QQQ : not correct. I will fix it later.
*/
static inline uint64_t local_cmp_fast(const char *p, const char *q, int n)
{
	__m128i x = load16(p, n);
	__m128i mask = _mm_loadu_si128((const __m128i*)(maskTbl + n));
	__m128i y = _mm_loadu_si128((const __m128i*)q);
	y = _mm_and_si128(y, mask);
	return _mm_testc_si128(x, y) == 0;
}
static inline uint64_t local_cmp3s(const char *p, const char *q)
{
	return local_cmp_fast(p, q, 3);
}
static inline uint64_t local_cmp5s(const char *p, const char *q)
{
	return local_cmp_fast(p, q, 5);
}
static inline uint64_t local_cmp6s(const char *p, const char *q)
{
	return local_cmp_fast(p, q, 6);
}
static inline uint64_t local_cmp7s(const char *p, const char *q)
{
	return local_cmp_fast(p, q, 7);
}
static inline uint64_t local_cmp9s(const char *p, const char *q)
{
	return local_cmp_fast(p, q, 9);
}
static inline uint64_t local_cmp10s(const char *p, const char *q)
{
	return local_cmp_fast(p, q, 10);
}
static inline uint64_t local_cmp11s(const char *p, const char *q)
{
	return local_cmp_fast(p, q, 11);
}
static inline uint64_t local_cmp12s(const char *p, const char *q)
{
	return local_cmp_fast(p, q, 12);
}
static inline uint64_t local_cmp13s(const char *p, const char *q)
{
	return local_cmp_fast(p, q, 13);
}
static inline uint64_t local_cmp14s(const char *p, const char *q)
{
	return local_cmp_fast(p, q, 14);
}
static inline uint64_t local_cmp15s(const char *p, const char *q)
{
	return local_cmp_fast(p, q, 15);
}

static inline uint64_t local_cmp3_memcmp(const char *p, const char *q) { return memcmp(p, q, 3); }
static inline uint64_t local_cmp4_memcmp(const char *p, const char *q) { return memcmp(p, q, 4); }
static inline uint64_t local_cmp7_memcmp(const char *p, const char *q) { return memcmp(p, q, 7); }
static inline uint64_t local_cmp15_memcmp(const char *p, const char *q) { return memcmp(p, q, 15); }

const int N = 10000;

void test()
{
	const char p[] = "abcdeababababababababbabaabcadabaabbabeabcdababeaec";
	int ret = 0;
	ret |= local_cmp1(p, p);
	ret |= local_cmp2(p, p);
	ret |= local_cmp3(p, p);
	ret |= local_cmp4(p, p);
	ret |= local_cmp5(p, p);
	ret |= local_cmp6(p, p);
	ret |= local_cmp7(p, p);
	ret |= local_cmp8(p, p);
	ret |= local_cmp9(p, p);
	ret |= local_cmp10(p, p);
	ret |= local_cmp11(p, p);
	ret |= local_cmp12(p, p);
	ret |= local_cmp13(p, p);
	ret |= local_cmp14(p, p);
	ret |= local_cmp15(p, p);
	ret |= local_cmp16(p, p);
	ret |= local_cmp17(p, p);
	ret |= local_cmp18(p, p);
	ret |= local_cmp19(p, p);
	ret |= local_cmp20(p, p);
	ret |= local_cmp21(p, p);
	ret |= local_cmp22(p, p);
	ret |= local_cmp23(p, p);
	ret |= local_cmp24(p, p);
	ret |= local_cmp25(p, p);
	ret |= local_cmp26(p, p);
	if (ret) puts("ERR equal");
	const char *q = p + 1;
	ret = -1;
	ret &= local_cmp1(p, q);
	ret &= local_cmp2(p, q);
	ret &= local_cmp3(p, q);
	ret &= local_cmp4(p, q);
	ret &= local_cmp5(p, q);
	ret &= local_cmp6(p, q);
	ret &= local_cmp7(p, q);
	ret &= local_cmp8(p, q);
	ret &= local_cmp9(p, q);
	ret &= local_cmp10(p, q);
	ret &= local_cmp11(p, q);
	ret &= local_cmp12(p, q);
	ret &= local_cmp13(p, q);
	ret &= local_cmp14(p, q);
	ret &= local_cmp15(p, q);
	ret &= local_cmp16(p, q);
	ret &= local_cmp17(p, q);
	ret &= local_cmp18(p, q);
	ret &= local_cmp19(p, q);
	ret &= local_cmp20(p, q);
	ret &= local_cmp21(p, q);
	ret &= local_cmp22(p, q);
	ret &= local_cmp23(p, q);
	ret &= local_cmp24(p, q);
	ret &= local_cmp25(p, q);
	ret &= local_cmp26(p, q);
	if (!ret) puts("ERR equal");
	puts("ok");
}
int main()
{
	test();
	cybozu::XorShift rg;
	std::string v;
	v.resize(N + 32);
#if 1
	for (int i = 0; i < N; i++) {
		v[i] = (uint8_t)('a');
	}
	const char str[128] = "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa";
#else
	for (int i = 0; i < N; i++) {
		v[i] = (uint8_t)('a' + (rg.get32() % 5));
	}
	const char str[128] = "abcdeababababababababbabaabcadabaabbabeabcdababeaec";
#endif
	size_t pos = 0;
	const char*p = v.data();
	volatile uint64_t ret = 0;
	CYBOZU_BENCH("1  ", ++pos %= N; ret += !!local_cmp1, &p[pos], str); pos = 0;
	CYBOZU_BENCH("2  ", ++pos %= N; ret += !!local_cmp2, &p[pos], str); pos = 0;
	CYBOZU_BENCH("2u ", ++pos %= N; ret += !!local_cmp2u, &p[pos], str); pos = 0;
	CYBOZU_BENCH("3  ", ++pos %= N; ret += !!local_cmp3, &p[pos], str); pos = 0;
	CYBOZU_BENCH("3u ", ++pos %= N; ret += !!local_cmp3u, &p[pos], str); pos = 0;
	CYBOZU_BENCH("3s ", ++pos %= N; ret += !!local_cmp3s, &p[pos], str); pos = 0;
	CYBOZU_BENCH("4  ", ++pos %= N; ret += !!local_cmp4, &p[pos], str); pos = 0;
	CYBOZU_BENCH("5  ", ++pos %= N; ret += !!local_cmp5, &p[pos], str); pos = 0;
	CYBOZU_BENCH("5u ", ++pos %= N; ret += !!local_cmp5u, &p[pos], str); pos = 0;
	CYBOZU_BENCH("5s ", ++pos %= N; ret += !!local_cmp5s, &p[pos], str); pos = 0;
	CYBOZU_BENCH("6  ", ++pos %= N; ret += !!local_cmp6, &p[pos], str); pos = 0;
	CYBOZU_BENCH("6u ", ++pos %= N; ret += !!local_cmp6u, &p[pos], str); pos = 0;
	CYBOZU_BENCH("6s ", ++pos %= N; ret += !!local_cmp6s, &p[pos], str); pos = 0;
	CYBOZU_BENCH("7  ", ++pos %= N; ret += !!local_cmp7, &p[pos], str); pos = 0;
	CYBOZU_BENCH("7u ", ++pos %= N; ret += !!local_cmp7u, &p[pos], str); pos = 0;
	CYBOZU_BENCH("7s ", ++pos %= N; ret += !!local_cmp7s, &p[pos], str); pos = 0;
	CYBOZU_BENCH("8  ", ++pos %= N; ret += !!local_cmp8, &p[pos], str); pos = 0;
	CYBOZU_BENCH("9  ", ++pos %= N; ret += !!local_cmp9, &p[pos], str); pos = 0;
	CYBOZU_BENCH("9s ", ++pos %= N; ret += !!local_cmp9s, &p[pos], str); pos = 0;
	CYBOZU_BENCH("10 ", ++pos %= N; ret += !!local_cmp10, &p[pos], str); pos = 0;
	CYBOZU_BENCH("10u", ++pos %= N; ret += !!local_cmp10u, &p[pos], str); pos = 0;
	CYBOZU_BENCH("10s", ++pos %= N; ret += !!local_cmp10s, &p[pos], str); pos = 0;
	CYBOZU_BENCH("11 ", ++pos %= N; ret += !!local_cmp11, &p[pos], str); pos = 0;
	CYBOZU_BENCH("11u", ++pos %= N; ret += !!local_cmp11u, &p[pos], str); pos = 0;
	CYBOZU_BENCH("11s", ++pos %= N; ret += !!local_cmp11u, &p[pos], str); pos = 0;
	CYBOZU_BENCH("12 ", ++pos %= N; ret += !!local_cmp12, &p[pos], str); pos = 0;
	CYBOZU_BENCH("12u", ++pos %= N; ret += !!local_cmp12u, &p[pos], str); pos = 0;
	CYBOZU_BENCH("12s", ++pos %= N; ret += !!local_cmp12s, &p[pos], str); pos = 0;
	CYBOZU_BENCH("13 ", ++pos %= N; ret += !!local_cmp13, &p[pos], str); pos = 0;
	CYBOZU_BENCH("13u", ++pos %= N; ret += !!local_cmp13u, &p[pos], str); pos = 0;
	CYBOZU_BENCH("13s", ++pos %= N; ret += !!local_cmp13s, &p[pos], str); pos = 0;
	CYBOZU_BENCH("14 ", ++pos %= N; ret += !!local_cmp14, &p[pos], str); pos = 0;
	CYBOZU_BENCH("14u", ++pos %= N; ret += !!local_cmp14u, &p[pos], str); pos = 0;
	CYBOZU_BENCH("14s", ++pos %= N; ret += !!local_cmp14s, &p[pos], str); pos = 0;
	CYBOZU_BENCH("15 ", ++pos %= N; ret += !!local_cmp15, &p[pos], str); pos = 0;
	CYBOZU_BENCH("15u", ++pos %= N; ret += !!local_cmp15u, &p[pos], str); pos = 0;
	CYBOZU_BENCH("15s", ++pos %= N; ret += !!local_cmp15s, &p[pos], str); pos = 0;
	CYBOZU_BENCH("16 ", ++pos %= N; ret += !!local_cmp16, &p[pos], str); pos = 0;
	CYBOZU_BENCH("20 ", ++pos %= N; ret += !!local_cmp20, &p[pos], str); pos = 0;
	CYBOZU_BENCH("get3 ", ++pos %= N; ret += !!local_get3, &p[pos]); pos = 0;
	CYBOZU_BENCH("get3f", ++pos %= N; ret += !!local_get3f, &p[pos]); pos = 0;
	CYBOZU_BENCH("memcmp4", ++pos %= N; ret += !!local_cmp4_memcmp, &p[pos], str); pos = 0;
	CYBOZU_BENCH("memcmp7", ++pos %= N; ret += !!local_cmp7_memcmp, &p[pos], str); pos = 0;
	CYBOZU_BENCH("memcmp15", ++pos %= N; ret += !!local_cmp15_memcmp, &p[pos], str); pos = 0;
	ret = 0;
	CYBOZU_BENCH_C("3", 2*N,++pos %= N; ret += !!local_cmp3, &p[pos], str); pos = 0;
	printf("ret=%d\n", (int)ret);
	ret = 0;
	CYBOZU_BENCH_C("u3", 2*N,++pos %= N; ret += !!local_cmp3u, &p[pos], str); pos = 0;
	printf("ret=%d\n", (int)ret);
	ret = 0;
	CYBOZU_BENCH_C("s3", 2*N,++pos %= N; ret += !!local_cmp3s, &p[pos], str); pos = 0;
	printf("ret=%d\n", (int)ret);
}
