#include "guicall.h"

__attribute__((noinline))
int64_t _guicall_impl(int64_t num, int64_t arg1, int64_t arg2, int64_t arg3,
                      int64_t arg4, int64_t arg5, int64_t arg6)
{
    int64_t resultado;
    
    register int64_t r10 asm("r10") = arg4;
    register int64_t r8 asm("r8") = arg5;
    register int64_t r9 asm("r9") = arg6;
    
    __asm__ volatile (
        "syscall"
        : "=a" (resultado)      
        : "a" (num),             
          "D" (arg1),            
          "S" (arg2),            
          "d" (arg3),            
          "r" (r10),             
          "r" (r8),              
          "r" (r9)               
        : "rcx", "r11", "memory" 
    );
    
    return resultado;
}
