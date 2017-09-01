#include <stdint.h>
#include <stdio.h>

extern "C" {

uint32_t add(uint32_t x, uint32_t y)
{
	return x + y;
}

uint32_t str2int(const uint8_t *s)
{
	return s[0] + s[1] * (1 << 8) + s[2] * (1 << 16) + s[3] * (1 << 24);
}

int int2str(char *buf, size_t maxBufSize, int v)
{
	return snprintf(buf, maxBufSize, "%d", v);
}

} // extern "C"