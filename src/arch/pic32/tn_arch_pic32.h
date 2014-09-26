/*******************************************************************************
 *
 * TNeoKernel: real-time kernel initially based on TNKernel
 *
 *    TNKernel:                  copyright � 2004, 2013 Yuri Tiomkin.
 *    PIC32-specific routines:   copyright � 2013, 2014 Anders Montonen.
 *    TNeoKernel:                copyright � 2014       Dmitry Frank.
 *
 *    TNeoKernel was born as a thorough review and re-implementation of
 *    TNKernel. The new kernel has well-formed code, inherited bugs are fixed
 *    as well as new features being added, and it is tested carefully with
 *    unit-tests.
 *
 *    API is changed somewhat, so it's not 100% compatible with TNKernel,
 *    hence the new name: TNeoKernel.
 *
 *    Permission to use, copy, modify, and distribute this software in source
 *    and binary forms and its documentation for any purpose and without fee
 *    is hereby granted, provided that the above copyright notice appear
 *    in all copies and that both that copyright notice and this permission
 *    notice appear in supporting documentation.
 *
 *    THIS SOFTWARE IS PROVIDED BY THE DMITRY FRANK AND CONTRIBUTORS "AS IS"
 *    AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 *    IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 *    PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL DMITRY FRANK OR CONTRIBUTORS BE
 *    LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 *    CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 *    SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 *    INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 *    CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 *    ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 *    THE POSSIBILITY OF SUCH DAMAGE.
 *
 ******************************************************************************/

/**
 *
 * \file
 *
 * PIC32 architecture-dependent routines
 *
 */


#ifndef  _TN_ARCH_PIC32_H
#define  _TN_ARCH_PIC32_H


#ifdef __cplusplus
extern "C"  {     /*}*/
#endif

#ifndef DOXYGEN_SHOULD_SKIP_THIS

/**
 * FFS - find first set bit. Used in `_find_next_task_to_run()` function.
 *
 * May be not defined: in this case, naive algorithm will be used.
 */
#define  _TN_FFS(x) (32 - __builtin_clz((x) & (0 - (x))))

/**
 * Used by the kernel as a signal that something really bad happened
 * (indicating a bug in the kernel)
 *
 * Typically, set to assembler instruction that causes debugger to halt.
 */
#define  _TN_FATAL_ERROR(error_msg, ...)         \
   {__asm__ volatile(" sdbbp 0"); __asm__ volatile ("nop");}



/**
 * \def TN_ARCH_STK_ATTR_BEFORE
 *
 * Compiler-specific attribute that should be placed **before** declaration of
 * array used for stack. It is needed because there are often additional 
 * restrictions applied to alignment of stack, so, to meet them, stack arrays
 * need to be declared with these macros.
 *
 * @see TN_ARCH_STK_ATTR_AFTER
 */

/**
 * \def TN_ARCH_STK_ATTR_AFTER
 *
 * Compiler-specific attribute that should be placed **after** declaration of
 * array used for stack. It is needed because there are often additional 
 * restrictions applied to alignment of stack, so, to meet them, stack arrays
 * need to be declared with these macros.
 *
 * @see TN_ARCH_STK_ATTR_BEFORE
 */

#if defined (__XC32)
#  define TN_ARCH_STK_ATTR_BEFORE
#  define TN_ARCH_STK_ATTR_AFTER       __attribute__((aligned(0x8)))
#else
#  error "Unknown compiler"
#endif

/**
 * Minimum task's stack size, in words, not in bytes; includes a space for
 * context plus for parameters passed to task's body function.
 */
#define  TN_MIN_STACK_SIZE          36

/**
 * Width of `int` type.
 */
#define  TN_INT_WIDTH               32

/**
 * Size of CPU register. Usually it's `sizeof(int)` or `sizeof(void *)`
 */
#define  TN_ALIGN                   sizeof(void *)

/**
 * Number of priorities available, this value usually matches `TN_INT_WIDTH`.
 * For compatibility with all platforms, it's recommended to use only values
 * from 1 to 14, inclusive.
 *
 * @see `TN_INT_WIDTH`
 */
#define  TN_PRIORITIES_CNT          TN_INT_WIDTH

/**
 * Value for infinite waiting, usually matches `UINT_MAX`
 */
#define  TN_WAIT_INFINITE           0xFFFFFFFF

/**
 * Value for initializing the task's stack
 */
#define  TN_FILL_STACK_VAL          0xFEEDFACE




/**
 * Declares variable that is used by macros `TN_INT_DIS_SAVE()` and
 * `TN_INT_RESTORE()` for storing status register value.
 *
 * @see `TN_INT_DIS_SAVE()`
 * @see `TN_INT_RESTORE()`
 */
#define  TN_INTSAVE_DATA            int tn_save_status_reg = 0;

/**
 * The same as `TN_INTSAVE_DATA` but for using in ISR together with
 * `TN_INT_IDIS_SAVE()`, `TN_INT_IRESTORE()`.
 *
 * @see `TN_INT_IDIS_SAVE()`
 * @see `TN_INT_IRESTORE()`
 */
#define  TN_INTSAVE_DATA_INT        TN_INTSAVE_DATA

/**
 * \def TN_INT_DIS_SAVE()
 *
 * Disable interrupts and return previous value of status register,
 * atomically. Similar `tn_arch_sr_save_int_dis()`, but implemented
 * as a macro, so it is potentially faster.
 *
 * Uses `TN_INTSAVE_DATA` as a temporary storage.
 *
 * @see `TN_INTSAVE_DATA`
 * @see `tn_arch_sr_save_int_dis()`
 */

/**
 * \def TN_INT_RESTORE()
 *
 * Restore previously saved status register.
 * Similar to `tn_arch_sr_restore()`, but implemented as a macro,
 * so it is potentially faster.
 *
 * Uses `TN_INTSAVE_DATA` as a temporary storage.
 *
 * @see `TN_INTSAVE_DATA`
 * @see `tn_arch_sr_save_int_dis()`
 */

#ifdef __mips16
#  define TN_INT_DIS_SAVE()   tn_save_status_reg = tn_arch_sr_save_int_dis()
#  define TN_INT_RESTORE()    tn_arch_sr_restore(tn_save_status_reg)
#else
#  define TN_INT_DIS_SAVE()   __asm__ __volatile__("di %0; ehb" : "=d" (tn_save_status_reg))
#  define TN_INT_RESTORE()    __builtin_mtc0(12, 0, tn_save_status_reg)
#endif

/**
 * The same as `TN_INT_DIS_SAVE()` but for using in ISR.
 *
 * Uses `TN_INTSAVE_DATA_INT` as a temporary storage.
 *
 * @see `TN_INTSAVE_DATA_INT`
 */
#define TN_INT_IDIS_SAVE()       TN_INT_DIS_SAVE()

/**
 * The same as `TN_INT_RESTORE()` but for using in ISR.
 *
 * Uses `TN_INTSAVE_DATA_INT` as a temporary storage.
 *
 * @see `TN_INTSAVE_DATA_INT`
 */
#define TN_INT_IRESTORE()        TN_INT_RESTORE()

/**
 * Returns nonzero if interrupts are disabled, zero otherwise.
 */
#define TN_IS_INT_DISABLED()     ((__builtin_mfc0(12, 0) & 1) == 0)





#define  TN_CHECK_INT_CONTEXT                      \
             if(!_tn_arch_inside_isr())            \
                return TN_RC_WCONTEXT;

#define  TN_CHECK_INT_CONTEXT_NORETVAL             \
             if(!_tn_arch_inside_isr())            \
                return;

#define  TN_CHECK_NON_INT_CONTEXT                  \
             if(_tn_arch_inside_isr())             \
                return TN_RC_WCONTEXT;

#define  TN_CHECK_NON_INT_CONTEXT_NORETVAL         \
             if(_tn_arch_inside_isr())             \
                return ;




#endif   //-- DOXYGEN_SHOULD_SKIP_THIS


// ---------------------------------------------------------------------------

/**
 * Interrupt handler wrapper macro for software context saving.
 *
 * Usage looks like the following:
 *
 *     tn_soft_isr(_TIMER_1_VECTOR)
 *     {
 *        INTClearFlag(INT_T1);
 *
 *        //-- do something useful
 *     }
 *
 * Note that you should not use `__ISR(_TIMER_1_VECTOR)` macro for that.
 *
 * @param vec  interrupt vector number, such as `_TIMER_1_VECTOR`, etc.
 */
#define tn_soft_isr(vec)                                                       \
__attribute__((__noinline__)) void _func##vec(void);                           \
void __attribute__((naked, nomips16))                                          \
     __attribute__((vector(vec)))                                              \
     _isr##vec(void)                                                           \
{                                                                              \
   asm volatile(".set mips32r2");                                              \
   asm volatile(".set nomips16");                                              \
   asm volatile(".set noreorder");                                             \
   asm volatile(".set noat");                                                  \
                                                                               \
   asm volatile("rdpgpr  $sp, $sp");                                           \
                                                                               \
   /* Increase interrupt nesting count */                                      \
   asm volatile("lui     $k0, %hi(tn_int_nest_count)");                        \
   asm volatile("lw      $k1, %lo(tn_int_nest_count)($k0)");                   \
   asm volatile("addiu   $k1, $k1, 1");                                        \
   asm volatile("sw      $k1, %lo(tn_int_nest_count)($k0)");                   \
   asm volatile("ori     $k0, $zero, 1");                                      \
   asm volatile("bne     $k1, $k0, 1f");                                       \
                                                                               \
   /* Swap stack pointers if nesting count is one */                           \
   asm volatile("lui     $k0, %hi(tn_user_sp)");                               \
   asm volatile("sw      $sp, %lo(tn_user_sp)($k0)");                          \
   asm volatile("lui     $k0, %hi(tn_int_sp)");                                \
   asm volatile("lw      $sp, %lo(tn_int_sp)($k0)");                           \
                                                                               \
   asm volatile("1:");                                                         \
   /* Save context on stack */                                                 \
   asm volatile("addiu   $sp, $sp, -92");                                      \
   asm volatile("mfc0    $k1, $14");               /* c0_epc*/                 \
   asm volatile("mfc0    $k0, $12, 2");            /* c0_srsctl*/              \
   asm volatile("sw      $k1, 84($sp)");                                       \
   asm volatile("sw      $k0, 80($sp)");                                       \
   asm volatile("mfc0    $k1, $12");               /* c0_status*/              \
   asm volatile("sw      $k1, 88($sp)");                                       \
                                                                               \
   /* Enable nested interrupts */                                              \
   asm volatile("mfc0    $k0, $13");               /* c0_cause*/               \
   asm volatile("ins     $k1, $zero, 1, 15");                                  \
   asm volatile("ext     $k0, $k0, 10, 6");                                    \
   asm volatile("ins     $k1, $k0, 10, 6");                                    \
   asm volatile("mtc0    $k1, $12");               /* c0_status*/              \
                                                                               \
   /* Save caller-save registers on stack */                                   \
   asm volatile("sw      $ra, 76($sp)");                                       \
   asm volatile("sw      $t9, 72($sp)");                                       \
   asm volatile("sw      $t8, 68($sp)");                                       \
   asm volatile("sw      $t7, 64($sp)");                                       \
   asm volatile("sw      $t6, 60($sp)");                                       \
   asm volatile("sw      $t5, 56($sp)");                                       \
   asm volatile("sw      $t4, 52($sp)");                                       \
   asm volatile("sw      $t3, 48($sp)");                                       \
   asm volatile("sw      $t2, 44($sp)");                                       \
   asm volatile("sw      $t1, 40($sp)");                                       \
   asm volatile("sw      $t0, 36($sp)");                                       \
   asm volatile("sw      $a3, 32($sp)");                                       \
   asm volatile("sw      $a2, 28($sp)");                                       \
   asm volatile("sw      $a1, 24($sp)");                                       \
   asm volatile("sw      $a0, 20($sp)");                                       \
   asm volatile("sw      $v1, 16($sp)");                                       \
   asm volatile("sw      $v0, 12($sp)");                                       \
   asm volatile("sw      $at, 8($sp)");                                        \
   asm volatile("mfhi    $v0");                                                \
   asm volatile("mflo    $v1");                                                \
   asm volatile("sw      $v0, 4($sp)");                                        \
                                                                               \
   /* Call ISR */                                                              \
   asm volatile("la      $t0, _func"#vec);                                     \
   asm volatile("jalr    $t0");                                                \
   asm volatile("sw      $v1, 0($sp)");                                        \
                                                                               \
   /* Pend context switch if needed */                                         \
   asm volatile("lw      $t0, tn_curr_run_task");                              \
   asm volatile("lw      $t1, tn_next_task_to_run");                           \
   asm volatile("lw      $t0, 0($t0)");                                        \
   asm volatile("lw      $t1, 0($t1)");                                        \
   asm volatile("lui     $t2, %hi(IFS0SET)");                                  \
   asm volatile("beq     $t0, $t1, 1f");                                       \
   asm volatile("ori     $t1, $zero, 2");                                      \
   asm volatile("sw      $t1, %lo(IFS0SET)($t2)");                             \
                                                                               \
   asm volatile("1:");                                                         \
   /* Restore registers */                                                     \
   asm volatile("lw      $v1, 0($sp)");                                        \
   asm volatile("lw      $v0, 4($sp)");                                        \
   asm volatile("mtlo    $v1");                                                \
   asm volatile("mthi    $v0");                                                \
   asm volatile("lw      $at, 8($sp)");                                        \
   asm volatile("lw      $v0, 12($sp)");                                       \
   asm volatile("lw      $v1, 16($sp)");                                       \
   asm volatile("lw      $a0, 20($sp)");                                       \
   asm volatile("lw      $a1, 24($sp)");                                       \
   asm volatile("lw      $a2, 28($sp)");                                       \
   asm volatile("lw      $a3, 32($sp)");                                       \
   asm volatile("lw      $t0, 36($sp)");                                       \
   asm volatile("lw      $t1, 40($sp)");                                       \
   asm volatile("lw      $t2, 44($sp)");                                       \
   asm volatile("lw      $t3, 48($sp)");                                       \
   asm volatile("lw      $t4, 52($sp)");                                       \
   asm volatile("lw      $t5, 56($sp)");                                       \
   asm volatile("lw      $t6, 60($sp)");                                       \
   asm volatile("lw      $t7, 64($sp)");                                       \
   asm volatile("lw      $t8, 68($sp)");                                       \
   asm volatile("lw      $t9, 72($sp)");                                       \
   asm volatile("lw      $ra, 76($sp)");                                       \
                                                                               \
   asm volatile("di");                                                         \
   asm volatile("ehb");                                                        \
                                                                               \
   /* Restore context */                                                       \
   asm volatile("lw      $k0, 84($sp)");                                       \
   asm volatile("mtc0    $k0, $14");               /* c0_epc */                \
   asm volatile("lw      $k0, 80($sp)");                                       \
   asm volatile("mtc0    $k0, $12, 2");            /* c0_srsctl */             \
   asm volatile("addiu   $sp, $sp, 92");                                       \
                                                                               \
   /* Decrease interrupt nesting count */                                      \
   asm volatile("lui     $k0, %hi(tn_int_nest_count)");                        \
   asm volatile("lw      $k1, %lo(tn_int_nest_count)($k0)");                   \
   asm volatile("addiu   $k1, $k1, -1");                                       \
   asm volatile("sw      $k1, %lo(tn_int_nest_count)($k0)");                   \
   asm volatile("bne     $k1, $zero, 1f");                                     \
   asm volatile("lw      $k1, -4($sp)");                                       \
                                                                               \
   /* Swap stack pointers if nesting count is zero */                          \
   asm volatile("lui     $k0, %hi(tn_int_sp)");                                \
   asm volatile("sw      $sp, %lo(tn_int_sp)($k0)");                           \
   asm volatile("lui     $k0, %hi(tn_user_sp)");                               \
   asm volatile("lw      $sp, %lo(tn_user_sp)($k0)");                          \
                                                                               \
   asm volatile("1:");                                                         \
   asm volatile("wrpgpr  $sp, $sp");                                           \
   asm volatile("mtc0    $k1, $12");               /* c0_status */             \
   asm volatile("eret");                                                       \
                                                                               \
   asm volatile(".set reorder");                                               \
   asm volatile(".set at");                                                    \
                                                                               \
} __attribute((__noinline__)) void _func##vec(void)




/**
 * Interrupt handler wrapper macro for shadow register context saving.
 *
 * Usage looks like the following:
 *
 *     tn_srs_isr(_INT_UART_1_VECTOR)
 *     {
 *        INTClearFlag(INT_U1);
 *
 *        //-- do something useful
 *     }
 *
 * Note that you should not use `__ISR(_INT_UART_1_VECTOR)` macro for that.
 *
 * @param vec  interrupt vector number, such as `_TIMER_1_VECTOR`, etc.
 */
#define tn_srs_isr(vec)                                                        \
__attribute__((__noinline__)) void _func##vec(void);                           \
void __attribute__((naked, nomips16))                                          \
     __attribute__((vector(vec)))                                              \
     _isr##vec(void)                                                           \
{                                                                              \
   asm volatile(".set mips32r2");                                              \
   asm volatile(".set nomips16");                                              \
   asm volatile(".set noreorder");                                             \
   asm volatile(".set noat");                                                  \
                                                                               \
   asm volatile("rdpgpr  $sp, $sp");                                           \
                                                                               \
   /* Increase interrupt nesting count */                                      \
   asm volatile("lui     $k0, %hi(tn_int_nest_count)");                        \
   asm volatile("lw      $k1, %lo(tn_int_nest_count)($k0)");                   \
   asm volatile("addiu   $k1, $k1, 1");                                        \
   asm volatile("sw      $k1, %lo(tn_int_nest_count)($k0)");                   \
   asm volatile("ori     $k0, $zero, 1");                                      \
   asm volatile("bne     $k1, $k0, 1f");                                       \
                                                                               \
   /* Swap stack pointers if nesting count is one */                           \
   asm volatile("lui     $k0, %hi(tn_user_sp)");                               \
   asm volatile("sw      $sp, %lo(tn_user_sp)($k0)");                          \
   asm volatile("lui     $k0, %hi(tn_int_sp)");                                \
   asm volatile("lw      $sp, %lo(tn_int_sp)($k0)");                           \
                                                                               \
   asm volatile("1:");                                                         \
   /* Save context on stack */                                                 \
   asm volatile("addiu   $sp, $sp, -20");                                      \
   asm volatile("mfc0    $k1, $14");               /* c0_epc */                \
   asm volatile("mfc0    $k0, $12, 2");            /* c0_srsctl */             \
   asm volatile("sw      $k1, 12($sp)");                                       \
   asm volatile("sw      $k0, 8($sp)");                                        \
   asm volatile("mfc0    $k1, $12");               /* c0_status */             \
   asm volatile("sw      $k1, 16($sp)");                                       \
                                                                               \
   /* Enable nested interrupts */                                              \
   asm volatile("mfc0    $k0, $13");               /* c0_cause */              \
   asm volatile("ins     $k1, $zero, 1, 15");                                  \
   asm volatile("ext     $k0, $k0, 10, 6");                                    \
   asm volatile("ins     $k1, $k0, 10, 6");                                    \
   asm volatile("mtc0    $k1, $12");               /* c0_status */             \
                                                                               \
   /* Save caller-save registers on stack */                                   \
   asm volatile("mfhi    $v0");                                                \
   asm volatile("mflo    $v1");                                                \
   asm volatile("sw      $v0, 4($sp)");                                        \
                                                                               \
   /* Call ISR */                                                              \
   asm volatile("la      $t0, _func"#vec);                                     \
   asm volatile("jalr    $t0");                                                \
   asm volatile("sw      $v1, 0($sp)");                                        \
                                                                               \
   /* Pend context switch if needed */                                         \
   asm volatile("lw      $t0, tn_curr_run_task");                              \
   asm volatile("lw      $t1, tn_next_task_to_run");                           \
   asm volatile("lw      $t0, 0($t0)");                                        \
   asm volatile("lw      $t1, 0($t1)");                                        \
   asm volatile("lui     $t2, %hi(IFS0SET)");                                  \
   asm volatile("beq     $t0, $t1, 1f");                                       \
   asm volatile("ori     $t1, $zero, 2");                                      \
   asm volatile("sw      $t1, %lo(IFS0SET)($t2)");                             \
                                                                               \
   asm volatile("1:");                                                         \
   /* Restore registers */                                                     \
   asm volatile("lw      $v1, 0($sp)");                                        \
   asm volatile("lw      $v0, 4($sp)");                                        \
   asm volatile("mtlo    $v1");                                                \
   asm volatile("mthi    $v0");                                                \
                                                                               \
   asm volatile("di");                                                         \
   asm volatile("ehb");                                                        \
                                                                               \
   /* Restore context */                                                       \
   asm volatile("lw      $k0, 12($sp)");                                       \
   asm volatile("mtc0    $k0, $14");               /* c0_epc */                \
   asm volatile("lw      $k0, 8($sp)");                                        \
   asm volatile("mtc0    $k0, $12, 2");            /* c0_srsctl */             \
   asm volatile("addiu   $sp, $sp, 20");                                       \
                                                                               \
   /* Decrease interrupt nesting count */                                      \
   asm volatile("lui     $k0, %hi(tn_int_nest_count)");                        \
   asm volatile("lw      $k1, %lo(tn_int_nest_count)($k0)");                   \
   asm volatile("addiu   $k1, $k1, -1");                                       \
   asm volatile("sw      $k1, %lo(tn_int_nest_count)($k0)");                   \
   asm volatile("bne     $k1, $zero, 1f");                                     \
   asm volatile("lw      $k1, -4($sp)");                                       \
                                                                               \
   /* Swap stack pointers if nesting count is zero */                          \
   asm volatile("lui     $k0, %hi(tn_int_sp)");                                \
   asm volatile("sw      $sp, %lo(tn_int_sp)($k0)");                           \
   asm volatile("lui     $k0, %hi(tn_user_sp)");                               \
   asm volatile("lw      $sp, %lo(tn_user_sp)($k0)");                          \
                                                                               \
   asm volatile("1:");                                                         \
   asm volatile("wrpgpr  $sp, $sp");                                           \
   asm volatile("mtc0    $k1, $12");               /* c0_status */             \
   asm volatile("eret");                                                       \
                                                                               \
   asm volatile(".set reorder");                                               \
   asm volatile(".set at");                                                    \
                                                                               \
} __attribute((__noinline__)) void _func##vec(void)


#ifdef __cplusplus
}  /* extern "C" */
#endif

#endif   // _TN_ARCH_PIC32_H

