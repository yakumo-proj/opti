
define i128 @mul64x64(i64 %x, i64 %y) {
	%x0 = zext i64 %x to i128
	%y0 = zext i64 %y to i128
	%z = mul i128 %x0, %y0
	ret i128 %z
}

define i64 @extract(i192 %x, i192 %shift) {
	%t0 = lshr i192 %x, %shift
	%t1 = trunc i192 %t0 to i64
	ret i64 %t1
}

define i256 @mul192x64(i192 %x, i64 %y) {
entry:
	%x0 = call i64 @extract(i192 %x, i192 0)
	%x1 = call i64 @extract(i192 %x, i192 64)
	%x2 = call i64 @extract(i192 %x, i192 128)

	%x0y = call i128 @mul64x64(i64 %x0, i64 %y)
	%x1y = call i128 @mul64x64(i64 %x1, i64 %y)
	%x2y = call i128 @mul64x64(i64 %x2, i64 %y)

	%x0y0 = zext i128 %x0y to i256
	%x1y0 = zext i128 %x1y to i256
	%x2y0 = zext i128 %x2y to i256

	%x1y1 = shl i256 %x1y0, 64
	%x2y1 = shl i256 %x2y0, 128

	%t = add i256 %x0y0, %x1y1
	%z = add i256 %t, %x2y1
	ret i256 %z
}

; void mie::mul192x192(uint64_t *z, const uint64_t *x, const uint64_t *y);
define void @_ZN3mie10mul192x192EPmPKmS2_(i64* %pz, i192* %px, i192* %py) {
entry:
	%x = load i192* %px
	%y = load i192* %py
	%y0 = call i64 @extract(i192 %y, i192 0)
	%y1 = call i64 @extract(i192 %y, i192 64)
	%y2 = call i64 @extract(i192 %y, i192 128)

	%sum0 = call i256 @mul192x64(i192 %x, i64 %y0)

	%t0 = trunc i256 %sum0 to i64
	store i64 %t0, i64* %pz

	%s0 = lshr i256 %sum0, 64
	%xy1 = call i256 @mul192x64(i192 %x, i64 %y1)
	%sum1 = add i256 %s0, %xy1
	%z1 = getelementptr i64* %pz, i32 1

	%ts1 = trunc i256 %sum1 to i64
	store i64 %ts1, i64* %z1

	%s1 = lshr i256 %sum1, 64
	%xy2 = call i256 @mul192x64(i192 %x, i64 %y2)
	%sum2 = add i256 %s1, %xy2
	%z2 = getelementptr i64* %pz, i32 2

	%p = bitcast i64* %z2 to i256*
	store i256 %sum2, i256* %p
	ret void
}

; NIST_P192
; 0xfffffffffffffffffffffffffffffffeffffffffffffffff
; 
;       0                1                2
; ffffffffffffffff fffffffffffffffe ffffffffffffffff 
; 
; p = (1 << 192) - (1 << 64) - 1
; (1 << 192) % p = (1 << 64) + 1
; 
; L : 192bit
; Hi: 64bit
; x = [H:L] = [H2:H1:H0:L]
; mod p
;    x = L + H + (H << 64)
;      = L + H + [H1:H0:0] + H2 + (H2 << 64)
;[e:t] = L + H + [H1:H0:H2] + [H2:0] ; 2bit(e) over
;      = t + e + (e << 64)

; void mie::modNIST_P192(uint64_t *z, const uint64_t *x);
define void @_ZN3mie12modNIST_P192EPmPKm(i192* %out, i192* %px) {
entry:
	%L192 = load i192* %px
	%L = zext i192 %L192 to i256

	%pH = getelementptr i192* %px, i32 1
	%H192 = load i192* %pH
	%H = zext i192 %H192 to i256

	%H10_ = shl i192 %H192, 64
	%H10 = zext i192 %H10_ to i256

	%H2_ = call i64 @extract(i192 %H192, i192 128)
	%H2 = zext i64 %H2_ to i256
	%H102 = or i256 %H10, %H2

	%H2s = shl i256 %H2, 64

	%t0 = add i256 %L, %H
	%t1 = add i256 %t0, %H102
	%t2 = add i256 %t1, %H2s

	%e = lshr i256 %t2, 192
	%t3 = trunc i256 %t2 to i192
	%e1 = trunc i256 %e to i192


	%t4 = add i192 %t3, %e1
	%e2 = shl i192 %e1, 64
	%t5 = add i192 %t4, %e2

	store i192 %t5, i192* %out

	ret void
}
