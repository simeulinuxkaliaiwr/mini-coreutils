#ifndef SYS_GUICALL_H
#define SYS_GUICALL_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

int64_t _guicall_impl(int64_t num, 
                      int64_t arg1, int64_t arg2, int64_t arg3,
                      int64_t arg4, int64_t arg5, int64_t arg6);


#define _GUICALL_CONCAT(a, b) a ## b
#define _GUICALL_CONCAT_INNER(a, b) _GUICALL_CONCAT(a, b)

#define _GUICALL_NUM_ARGS(...) \
    _GUICALL_NUM_ARGS_IMPL(0, __VA_ARGS__, 7, 6, 5, 4, 3, 2, 1, 0)
    
#define _GUICALL_NUM_ARGS_IMPL(_0, _1, _2, _3, _4, _5, _6, _7, N, ...) N


#define _GUICALL_DISPATCH_1(name, N, ...) name##0(__VA_ARGS__) // N=1 (guicall0)
#define _GUICALL_DISPATCH_2(name, N, ...) name##1(__VA_ARGS__) // N=2 (guicall1)
#define _GUICALL_DISPATCH_3(name, N, ...) name##2(__VA_ARGS__) // N=3 (guicall2)
#define _GUICALL_DISPATCH_4(name, N, ...) name##3(__VA_ARGS__) // N=4 (guicall3)
#define _GUICALL_DISPATCH_5(name, N, ...) name##4(__VA_ARGS__) // N=5 (guicall4)
#define _GUICALL_DISPATCH_6(name, N, ...) name##5(__VA_ARGS__) // N=6 (guicall5)
#define _GUICALL_DISPATCH_7(name, N, ...) name##6(__VA_ARGS__) // N=7 (guicall6)

#define _GUICALL_OVERLOAD_FINAL(name, N, ...) \
    _GUICALL_CONCAT_INNER(_GUICALL_DISPATCH_, N)(name, N, __VA_ARGS__)



#define guicall0(num) _guicall_impl((int64_t)(num), 0, 0, 0, 0, 0, 0)
#define guicall1(num, a1) _guicall_impl((int64_t)(num), (int64_t)(a1), 0, 0, 0, 0, 0)
#define guicall2(num, a1, a2) _guicall_impl((int64_t)(num), (int64_t)(a1), (int64_t)(a2), 0, 0, 0, 0)
#define guicall3(num, a1, a2, a3) _guicall_impl((int64_t)(num), (int64_t)(a1), (int64_t)(a2), (int64_t)(a3), 0, 0, 0)
#define guicall4(num, a1, a2, a3, a4) _guicall_impl((int64_t)(num), (int64_t)(a1), (int64_t)(a2), (int64_t)(a3), (int64_t)(a4), 0, 0)
#define guicall5(num, a1, a2, a3, a4, a5) _guicall_impl((int64_t)(num), (int64_t)(a1), (int64_t)(a2), (int64_t)(a3), (int64_t)(a4), (int64_t)(a5), 0)
#define guicall6(num, a1, a2, a3, a4, a5, a6) _guicall_impl((int64_t)(num), (int64_t)(a1), (int64_t)(a2), (int64_t)(a3), (int64_t)(a4), (int64_t)(a5), (int64_t)(a6))


#define guicall(...) \
    _GUICALL_OVERLOAD_FINAL(guicall, _GUICALL_NUM_ARGS(__VA_ARGS__), __VA_ARGS__)


#ifdef __cplusplus
}
#endif

#endif // SYS_GUICALL_H
