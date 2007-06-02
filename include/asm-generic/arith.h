/**
 *   @ingroup hal
 *   @file
 *
 *   Generic arithmetic/conversion routines.
 *   Copyright &copy; 2005 Stelian Pop.
 *   Copyright &copy; 2005 Gilles Chanteperdrix.
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, Inc., 675 Mass Ave, Cambridge MA 02139,
 *   USA; either version 2 of the License, or (at your option) any later
 *   version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

/**
 * @addtogroup hal
 *@{*/

#ifndef _XENO_ASM_GENERIC_ARITH_H
#define _XENO_ASM_GENERIC_ARITH_H

#ifdef __KERNEL__
#include <asm/byteorder.h>
#include <asm/div64.h>

#ifdef __BIG_ENDIAN
#define endianstruct struct { unsigned _h; unsigned _l; } _s
#else /* __LITTLE_ENDIAN */
#define endianstruct struct { unsigned _l; unsigned _h; } _s
#endif

#else /* !__KERNEL__ */
#include <endian.h>

#if __BYTE_ORDER == __BIG_ENDIAN
#define endianstruct struct { unsigned _h; unsigned _l; } _s
#else /* __BYTE_ORDER == __LITTLE_ENDIAN */
#define endianstruct struct { unsigned _l; unsigned _h; } _s
#endif /* __BYTE_ORDER == __LITTLE_ENDIAN */

static inline unsigned __rthal_do_div(unsigned long long *a, unsigned d)
{
	unsigned r = *a % d;
	*a /= d;
	return r;
}

#define do_div(a, d) __rthal_do_div(&(a), (d))

#endif /* !__KERNEL__ */

#ifndef __rthal_u64tou32
#define __rthal_u64tou32(ull, h, l) ({          \
    union { unsigned long long _ull;            \
    endianstruct;                               \
    } _u;                                       \
    _u._ull = (ull);                            \
    (h) = _u._s._h;                             \
    (l) = _u._s._l;                             \
})
#endif /* !__rthal_u64tou32 */

#ifndef __rthal_u64fromu32
#define __rthal_u64fromu32(h, l) ({             \
    union { unsigned long long _ull;            \
    endianstruct;                               \
    } _u;                                       \
    _u._s._h = (h);                             \
    _u._s._l = (l);                             \
    _u._ull;                                    \
})
#endif /* !__rthal_u64fromu32 */

#ifndef rthal_ullmul
static inline __attribute__((__const__)) unsigned long long
__rthal_generic_ullmul(const unsigned m0, const unsigned m1)
{
    return (unsigned long long) m0 * m1;
}
#define rthal_ullmul(m0,m1) __rthal_generic_ullmul((m0),(m1))
#endif /* !rthal_ullmul */

#ifndef rthal_ulldiv
static inline unsigned long long __rthal_generic_ulldiv (unsigned long long ull,
                                                         const unsigned uld,
                                                         unsigned long *const rp)
{
    const unsigned r = do_div(ull, uld);

    if (rp)
        *rp = r;

    return ull;
}
#define rthal_ulldiv(ull,uld,rp) __rthal_generic_ulldiv((ull),(uld),(rp))
#endif /* !rthal_ulldiv */

#ifndef rthal_uldivrem
#define rthal_uldivrem(ull,ul,rp) ((unsigned) rthal_ulldiv((ull),(ul),(rp)))
#endif /* !rthal_uldivrem */

#ifndef rthal_imuldiv
static inline __attribute__((__const__)) int __rthal_generic_imuldiv (int i,
								  int mult,
								  int div)
{
    /* Returns (int)i = (unsigned long long)i*(unsigned)(mult)/(unsigned)div. */
    const unsigned long long ull = rthal_ullmul(i, mult);
    return rthal_uldivrem(ull, div, NULL);
}
#define rthal_imuldiv(i,m,d) __rthal_generic_imuldiv((i),(m),(d))
#endif /* !rthal_imuldiv */

#ifndef rthal_llimd
/* Division of an unsigned 96 bits ((h << 32) + l) by an unsigned 32 bits. 
   Building block for llimd. Without const qualifiers, gcc reload registers
   after each call to uldivrem. */
static inline unsigned long long
__rthal_generic_div96by32 (const unsigned long long h,
                           const unsigned l,
                           const unsigned d,
                           unsigned long *const rp)
{
    unsigned long rh;
    const unsigned qh = rthal_uldivrem(h, d, &rh);
    const unsigned long long t = __rthal_u64fromu32(rh, l);
    const unsigned ql = rthal_uldivrem(t, d, rp);

    return __rthal_u64fromu32(qh, ql);
}

static inline __attribute__((__const__))
unsigned long long __rthal_generic_ullimd (const unsigned long long op,
                                           const unsigned m,
                                           const unsigned d)
{
    unsigned oph, opl, tlh, tll;
    unsigned long long th, tl;
    
    __rthal_u64tou32(op, oph, opl);
    tl = rthal_ullmul(opl, m);
    __rthal_u64tou32(tl, tlh, tll);
    th = rthal_ullmul(oph, m);
    th += tlh;

    return __rthal_generic_div96by32(th, tll, d, NULL);
}

static inline __attribute__((__const__)) long long
__rthal_generic_llimd (long long op, unsigned m, unsigned d)
{

    if(op < 0LL)
        return -__rthal_generic_ullimd(-op, m, d);
    return __rthal_generic_ullimd(op, m, d);
}
#define rthal_llimd(ll,m,d) __rthal_generic_llimd((ll),(m),(d))
#endif /* !rthal_llimd */

#define xnarch_ullmod(ull,uld,rem)   ({ xnarch_ulldiv(ull,uld,rem); (*rem); })
#define xnarch_uldiv(ull, d)         rthal_uldivrem(ull, d, NULL)
#define xnarch_ulmod(ull, d)         ({ u_long _rem;                    \
                                        rthal_uldivrem(ull,d,&_rem); _rem; })

#define xnarch_ullmul                rthal_ullmul
#define xnarch_uldivrem              rthal_uldivrem
#define xnarch_ulldiv                rthal_ulldiv
#define xnarch_imuldiv               rthal_imuldiv
#define xnarch_llimd                 rthal_llimd
#define xnarch_get_cpu_tsc           rthal_rdtsc

/*@}*/

#endif /* _XENO_ASM_GENERIC_ARITH_H */