/*
 * plist_compat.h -- libplist 2.2 vs >=2.3 API shim.
 *
 * libplist 2.3.0 (2023-06) added plist_mem_free() and gained a 4th
 * plist_format_t* argument on plist_from_memory().  Debian bookworm,
 * Ubuntu 22.04, and other stable distros still ship 2.2.x, where
 * plist_from_memory() takes 3 args and plist_mem_free() does not exist.
 *
 * LIBPLIST_VERSION_* is not exposed by the public header on any released
 * version, so upstream-style version gating is unreliable.  The Makefile
 * feature-detects both symbols with a compile canary and passes
 * -DHAVE_PLIST_MEM_FREE / -DHAVE_PLIST_FROM_MEMORY_4ARG.  This header
 * keys on those to select the correct call form and provides a fallback
 * free() when the real symbol is missing.
 */

#ifndef TR4MPASS_PLIST_COMPAT_H
#define TR4MPASS_PLIST_COMPAT_H

#include <stdint.h>
#include <stdlib.h>
#include <plist/plist.h>

/* Default to the modern (>=2.3) API if the Makefile did not probe. */
#ifndef HAVE_PLIST_MEM_FREE
#define HAVE_PLIST_MEM_FREE 1
#endif
#ifndef HAVE_PLIST_FROM_MEMORY_4ARG
#define HAVE_PLIST_FROM_MEMORY_4ARG 1
#endif

#if HAVE_PLIST_MEM_FREE == 0
/* Provide plist_mem_free() so existing call sites compile unchanged. */
#define plist_mem_free(p) free(p)
static inline void tp_plist_mem_free(void *p) { free(p); }
#else
static inline void tp_plist_mem_free(void *p) { plist_mem_free(p); }
#endif

/*
 * tp_plist_from_memory -- always-4-arg wrapper.  On libplist <2.3 the
 * format-out pointer is dropped; on >=2.3 it is passed through.  Return
 * value is intentionally void because every current call site ignores it.
 */
static inline void tp_plist_from_memory(const char *data,
                                        uint32_t length,
                                        plist_t *out,
                                        void *format_out)
{
#if HAVE_PLIST_FROM_MEMORY_4ARG == 0
    (void)format_out;
    plist_from_memory(data, length, out);
#else
    plist_from_memory(data, length, out, (plist_format_t *)format_out);
#endif
}

#endif /* TR4MPASS_PLIST_COMPAT_H */
