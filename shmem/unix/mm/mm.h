/* ====================================================================
 * Copyright (c) 1999 Ralf S. Engelschall. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer. 
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 *
 * 3. All advertising materials mentioning features or use of this
 *    software must display the following acknowledgment:
 *    "This product includes software developed by
 *     Ralf S. Engelschall <rse@engelschall.com>."
 *
 * 4. Redistributions of any form whatsoever must retain the following
 *    acknowledgment:
 *    "This product includes software developed by
 *     Ralf S. Engelschall <rse@engelschall.com>."
 *
 * THIS SOFTWARE IS PROVIDED BY RALF S. ENGELSCHALL ``AS IS'' AND ANY
 * EXPRESSED OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL RALF S. ENGELSCHALL OR
 * ITS CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 * ====================================================================
 */

/*
**
**  mm.h -- Shared Memory library API header
**
*/

#ifndef MM_H 
#define MM_H 1

#ifdef __cplusplus
extern "C" {
#endif

/*
**  ____ Public Part (I) of the API ________________________
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

typedef enum { 
    MM_LOCK_RD, MM_LOCK_RW
} mm_lock_mode;

/*
**  ____ Private Part of the API ___________________________
*/

#if defined(MM_PRIVATE)

#include "mm_conf.h"

#include <errno.h>
#include <limits.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>

#ifdef MM_OS_SUNOS
#define KERNEL 1
#include <memory.h>
/* SunOS lacks prototypes */
extern int   getpagesize(void);
extern int   munmap(caddr_t, int);
extern int   ftruncate(int, off_t);
extern int   flock(int, int);
extern char *strerror(int);
#endif

#if !defined(APR_FALSE)
#define APR_FALSE 0
#endif
#if !defined(APR_TRUE)
#define APR_TRUE !APR_FALSE
#endif
#if !defined(NULL)
#define NULL (void *)0
#endif
#if !defined(NUL)
#define NUL '\0'
#endif
#if !defined(min_of)
#define min_of(a,b) ((a) < (b) ? (a) : (b))
#endif
#if !defined(max_of)
#define max_of(a,b) ((a) > (b) ? (a) : (b))
#endif
#if !defined(absof)
#define abs_of(a) ((a) < 0 ? -(a) : (a))
#endif
#if !defined(offset_of)
#define offset_of(type,member) ((size_t)(&((type *)0)->member))
#endif

#if !defined(HAVE_MEMCPY) 
#if defined(HAVE_BCOPY)
#define memcpy(to,from,len) bcopy(from,to,len)
#else
#define memcpy(to,from,len) \
        { int i; for (i = 0; i < (len); i++) *((to)+i) = *((from)+i); }
#endif
#endif
#if !defined(HAVE_MEMSET)
#define memset(to,ch,len) \
        { int i; for (i = 0; i < (len); i++) *((to)+i) = (ch); }
#endif

#define ERR(type,str)   mm_lib_error_set(type,str)
#define FAIL(type,str)  { ERR(type,str); goto cus; }
#define BEGIN_FAILURE   cus:
#define END_FAILURE   

#if defined(HAVE_PATH_MAX)
#define MM_MAXPATH PATH_MAX
#elif defined(HAVE__POSIX_PATH_MAX)
#define MM_MAXPATH _POSIX_PATH_MAX
#elif defined(HAVE_MAXPATHLEN)
#define MM_MAXPATH MAXPATHLEN
#else
#define MM_MAXPATH 2048
#endif

#if defined(HAVE_CHILD_MAX)
#define MM_MAXCHILD CHILD_MAX
#elif defined(HAVE__POSIX_CHILD_MAX)
#define MM_MAXCHILD _POSIX_CHILD_MAX
#else
#define MM_MAXCHILD 512
#endif

#if defined(MM_SHMT_MMANON) || defined(MM_SHMT_MMPOSX) ||\
    defined(MM_SHMT_MMZERO) || defined(MM_SHMT_MMFILE)
#include <sys/mman.h>
#if !defined(MAP_ANON) && defined(MAP_ANONYMOUS)
#define MAP_ANON MAP_ANONYMOUS
#endif
#if !defined(MAP_FAILED)
#define MAP_FAILED ((void *)-1)
#endif
#endif

#if defined(MM_SHMT_IPCSHM) || defined(MM_SEMT_IPCSEM)
#include <sys/ipc.h>
#endif

#if defined(MM_SHMT_IPCSHM)
#include <sys/shm.h>
#endif

#if defined(MM_SEMT_IPCSEM)
#include <sys/sem.h>
#ifndef HAVE_UNION_SEMUN
union semun {
    int val;
    struct semid_ds *buf;
    u_short *array;
};
#endif
#endif

#ifdef MM_SEMT_FLOCK
#include <sys/file.h>
#endif

#ifdef MM_SHMT_BEOS
#include <kernel/OS.h>
#endif

#define MM_ALLOC_MINSIZE         (1024*8) 
#define MM_CORE_FILEMODE         (S_IRUSR|S_IWUSR)
#define MM_CORE_DEFAULT_PAGESIZE (1024*8)
#define MM_CORE_DEFAULT_FILE     "/tmp/mm.core.%d"  /* %d is PID */

#define MM_ERR_ALLOC    1
#define MM_ERR_CORE     2
#define MM_ERR_SYSTEM   4

/* 
 * Define a union with types which are likely to have the longest
 * *relevant* CPU-specific memory word alignment restrictions...
 */
union mem_word {
    void  *mw_vp;
    void (*mw_fp)(void);
    char  *mw_cp;
    long   mw_l;
    double mw_d;
};
typedef union mem_word mem_word;
#define SIZEOF_mem_word (sizeof(mem_word))

/*
 * Define the structure used for memory chunks
 */
union mem_chunk_mc_u {
    struct mem_chunk *mc_next; /* really used when it's free         */
    mem_word          mc_base; /* virtually used when it's allocated */
};
struct mem_chunk {
    size_t mc_size;      /* physical size   */
    size_t mc_usize;     /* user known size */
    union mem_chunk_mc_u mc_u;
};
typedef struct mem_chunk mem_chunk;
#define SIZEOF_mem_chunk (sizeof(mem_chunk)-sizeof(union mem_chunk_mc_u))

/*
 * Define the structure describing a memory pool
 */
struct mem_pool {
   size_t     mp_size;
   size_t     mp_offset;
   mem_chunk  mp_freechunks;
   mem_word   mp_base;
};
typedef struct mem_pool mem_pool;
#define SIZEOF_mem_pool (sizeof(mem_pool)-SIZEOF_mem_word)

/*
 * Define the structure holding per-process filedescriptors
 */
#if defined(MM_SEMT_FLOCK)
struct mem_core_fd {
    pid_t pid;
    int fd;
};
typedef struct mem_core_fd mem_core_fd;
#define SIZEOF_mem_core_fd (sizeof(mem_core_fd)
#endif

/*
 * Define the structure describing a shared memory core area
 * (the actual contents depends on the shared memory and
 * semaphore/mutex type and is stripped down to a minimum
 * required)
 */
struct mem_core {
   size_t       mc_size;
   size_t       mc_usize;
   pid_t        mc_pid;
   int          mc_fdmem;
#if defined(MM_SHMT_MMFILE)
   char         mc_fnmem[MM_MAXPATH];
#endif
#if defined(MM_SHMT_BEOS)
    area_id     mc_areaid;
#endif
#if defined(MM_SEMT_BEOS)
    sem_id      mc_semid;
    int32       mc_ben;
#endif
#if !defined(MM_SEMT_FLOCK)
   int          mc_fdsem;
#endif
#if defined(MM_SEMT_FLOCK)
   mem_core_fd  mc_fdsem[MM_MAXCHILD];
#endif
#if defined(MM_SEMT_IPCSEM)
   int          mc_fdsem_rd;
   int          mc_readers;
   mm_lock_mode mc_lockmode;
#endif
#if defined(MM_SEMT_FLOCK) || defined(MM_SEMT_FCNTL)
   char         mc_fnsem[MM_MAXPATH];
#endif
   mem_word     mc_base;
};
typedef struct mem_core mem_core;
#define SIZEOF_mem_core (sizeof(mem_core)-SIZEOF_mem_word)

#endif /* MM_PRIVATE */

/*
**  ____ Public Part (II) of the API _______________________
*/

#if defined(MM_PRIVATE)
typedef mem_pool MM;
#else
typedef void MM;
#endif

/* Global Malloc-Replacement API */
int     MM_create(size_t size, const char *file);
int     MM_permission(mode_t mode, uid_t owner, gid_t group);
void    MM_destroy(void);
int     MM_lock(mm_lock_mode mode);
int     MM_unlock(void);
void   *MM_malloc(size_t size);
void   *MM_realloc(void *ptr, size_t size);
void    MM_free(void *ptr);
void   *MM_calloc(size_t number, size_t size);
char   *MM_strdup(const char *str);
size_t  MM_sizeof(const void *ptr);
size_t  MM_maxsize(void);
size_t  MM_available(void);
char   *MM_error(void);

/* Standard Malloc-Style API */
MM     *mm_create(size_t size, const char *file);
int     mm_permission(MM *mm, mode_t mode, uid_t owner, gid_t group);
void    mm_destroy(MM *mm);
int     mm_lock(MM *mm, mm_lock_mode mode);
int     mm_unlock(MM *mm);
void   *mm_malloc(MM *mm, size_t size);
void   *mm_realloc(MM *mm, void *ptr, size_t size);
void    mm_free(MM *mm, void *ptr);
void   *mm_calloc(MM *mm, size_t number, size_t size);
char   *mm_strdup(MM *mm, const char *str);
size_t  mm_sizeof(MM *mm, const void *ptr);
size_t  mm_maxsize(void);
size_t  mm_available(MM *mm);
char   *mm_error(void);
void    mm_display_info(MM *mm);

/* Low-Level Shared Memory API */
void   *mm_core_create(size_t size, const char *file);
int     mm_core_permission(void *core, mode_t mode, uid_t owner, gid_t group);
void    mm_core_delete(void *core);
size_t  mm_core_size(const void *core);
int     mm_core_lock(const void *core, mm_lock_mode mode);
int     mm_core_unlock(const void *core);
size_t  mm_core_maxsegsize(void);
size_t  mm_core_align2page(size_t size);
size_t  mm_core_align2word(size_t size);

/* Internal Library API */
void    mm_lib_error_set(unsigned int, const char *str);
char   *mm_lib_error_get(void);
int     mm_lib_version(void);

#ifdef __cplusplus
}
#endif

#endif /* MM_H */
