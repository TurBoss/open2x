/*
 * include/asm-alpha/cache.h
 */
#ifndef __ARCH_ALPHA_CACHE_H
#define __ARCH_ALPHA_CACHE_H


/* Bytes per L1 (data) cache line. */
#if defined(CONFIG_ALPHA_GENERIC) || defined(CONFIG_ALPHA_EV6) || defined(CONFIG_ALPHA_EV67)
# define L1_CACHE_BYTES     64
# define L1_CACHE_SHIFT     6
#else
/* Both EV4 and EV5 are write-through, read-allocate,
   direct-mapped, physical.
*/
# define L1_CACHE_BYTES     32
# define L1_CACHE_SHIFT     5
#endif

#define L1_CACHE_ALIGN(x)  (((x)+(L1_CACHE_BYTES-1))&~(L1_CACHE_BYTES-1))
#define SMP_CACHE_BYTES    L1_CACHE_BYTES
#define L1_CACHE_SHIFT_MAX L1_CACHE_SHIFT

#endif
