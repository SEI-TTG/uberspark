/*-
 * Copyright (c) 1990, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * Chris Torek.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */
/*
 * From: FreeBSD sys/libkern/strncpy.c
 */
/**
 * Modified for XMHF.
 */

#include <stdint.h>
#include <string.h>

/*@
	requires n >= 0;
	requires \exists integer i; Length_of_str_is(src, i);
	requires \valid(dst+(0..n-1));
	requires \valid(((char*)src)+(0..n-1));
	requires \separated(src+(0..n-1), dst+(0..n-1));
	assigns dst[0..n-1];
	ensures \result == dst;
@*/
char *strncpy(char *dst, const char *src, size_t n)
{
	char *q = dst;
	const char *p = src;
	char ch;
	size_t i;

	/*@
		loop invariant 0 <= n <= \at(n,Pre);
		loop invariant \at(n, Here) != \at(n, Pre) ==> ch != 0;
		loop invariant p == ((char*)src)+(\at(n, Pre) - n);
		loop invariant q == ((char*)dst)+(\at(n, Pre) - n);
		loop invariant (char*)dst <= q <= (char*)dst+\at(n,Pre);
		loop invariant (char*)src <= p <= (char*)src+\at(n,Pre);
		loop assigns n, q, ch, p, ((char*)dst)[0..(\at(n,Pre)- n - 1)];
		loop variant n;
	@*/
	while (n) {
		ch = *p;
		*q = ch;
		if (!ch)
			break;
		q++;
		p++;
		n--;
	}

	//memset(q, 0, n);
	/*@
		loop invariant 0 <= i <= n;
		loop assigns i;
		loop assigns q[0..(n-1)];
		loop variant n-i;
	@*/
	for(i=0; i < n; i++){
		q[i]=(char)0;
	}

	return dst;
}
