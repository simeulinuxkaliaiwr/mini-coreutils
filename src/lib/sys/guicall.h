#ifndef SYS_GUICALL_H
#define SYS_GUICALL_H

#if !defined(__x86_64__)
#error "This project only supports Linux x86_64"
#endif

#include "sysnums.h"

typedef signed long long int64_t;
typedef unsigned long long uint64_t;

#ifdef __cplusplus
extern "C" {
#endif


#define _guicall0(num) \
({ \
    int64_t _ret; \
    int64_t _num = (int64_t)(num); \
    __asm__ __volatile__ ( \
        "syscall" \
        : "=a" (_ret) \
        : "a" (_num) \
        : "rcx", "r11", "memory", "cc" \
    ); \
    _ret; \
})

#define _guicall1(num, a1) \
({ \
    int64_t _ret; \
    int64_t _num = (int64_t)(num); \
    int64_t _a1 = (int64_t)(a1); \
    __asm__ __volatile__ ( \
        "syscall" \
        : "=a" (_ret) \
        : "a" (_num), "D" (_a1) \
        : "rcx", "r11", "memory", "cc" \
    ); \
    _ret; \
})

#define _guicall2(num, a1, a2) \
({ \
    int64_t _ret; \
    int64_t _num = (int64_t)(num); \
    int64_t _a1 = (int64_t)(a1); \
    int64_t _a2 = (int64_t)(a2); \
    __asm__ __volatile__ ( \
        "syscall" \
        : "=a" (_ret) \
        : "a" (_num), "D" (_a1), "S" (_a2) \
        : "rcx", "r11", "memory", "cc" \
    ); \
    _ret; \
})

#define _guicall3(num, a1, a2, a3) \
({ \
    int64_t _ret; \
    int64_t _num = (int64_t)(num); \
    int64_t _a1 = (int64_t)(a1); \
    int64_t _a2 = (int64_t)(a2); \
    int64_t _a3 = (int64_t)(a3); \
    __asm__ __volatile__ ( \
        "syscall" \
        : "=a" (_ret) \
        : "a" (_num), "D" (_a1), "S" (_a2), "d" (_a3) \
        : "rcx", "r11", "memory", "cc" \
    ); \
    _ret; \
})

#define _guicall4(num, a1, a2, a3, a4) \
({ \
    int64_t _ret; \
    int64_t _num = (int64_t)(num); \
    int64_t _a1 = (int64_t)(a1); \
    int64_t _a2 = (int64_t)(a2); \
    int64_t _a3 = (int64_t)(a3); \
    register int64_t _r10 __asm__("r10") = (int64_t)(a4); \
    __asm__ __volatile__ ( \
        "syscall" \
        : "=a" (_ret) \
        : "a" (_num), "D" (_a1), "S" (_a2), "d" (_a3), "r" (_r10) \
        : "rcx", "r11", "memory", "cc" \
    ); \
    _ret; \
})

#define _guicall5(num, a1, a2, a3, a4, a5) \
({ \
    int64_t _ret; \
    int64_t _num = (int64_t)(num); \
    int64_t _a1 = (int64_t)(a1); \
    int64_t _a2 = (int64_t)(a2); \
    int64_t _a3 = (int64_t)(a3); \
    register int64_t _r10 __asm__("r10") = (int64_t)(a4); \
    register int64_t _r8  __asm__("r8")  = (int64_t)(a5); \
    __asm__ __volatile__ ( \
        "syscall" \
        : "=a" (_ret) \
        : "a" (_num), "D" (_a1), "S" (_a2), "d" (_a3), "r" (_r10), "r" (_r8) \
        : "rcx", "r11", "memory", "cc" \
    ); \
    _ret; \
})

#define _guicall6(num, a1, a2, a3, a4, a5, a6) \
({ \
    int64_t _ret; \
    int64_t _num = (int64_t)(num); \
    int64_t _a1 = (int64_t)(a1); \
    int64_t _a2 = (int64_t)(a2); \
    int64_t _a3 = (int64_t)(a3); \
    register int64_t _r10 __asm__("r10") = (int64_t)(a4); \
    register int64_t _r8  __asm__("r8")  = (int64_t)(a5); \
    register int64_t _r9  __asm__("r9")  = (int64_t)(a6); \
    __asm__ __volatile__ ( \
        "syscall" \
        : "=a" (_ret) \
        : "a" (_num), "D" (_a1), "S" (_a2), "d" (_a3), "r" (_r10), "r" (_r8), "r" (_r9) \
        : "rcx", "r11", "memory", "cc" \
    ); \
    _ret; \
})


#define _GUICALL_NARG_(_1,_2,_3,_4,_5,_6,_7,N,...) N
#define _GUICALL_NARG(...) _GUICALL_NARG_(__VA_ARGS__,6,5,4,3,2,1,0)

#define _GUICALL_CONCAT(x,y) x##y
#define _GUICALL_DISPATCH(n) _GUICALL_CONCAT(_guicall,n)

#define _GUICALL_APPLY(n,...) _GUICALL_DISPATCH(n)(__VA_ARGS__)

#define guicall(...) _GUICALL_APPLY(_GUICALL_NARG(__VA_ARGS__), __VA_ARGS__)

#ifdef __cplusplus
}
#endif

#endif /* SYS_GUICALL_H */
