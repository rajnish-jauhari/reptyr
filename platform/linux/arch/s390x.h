/*
 * Copyright (C) 2011 by Nelson Elhage
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

/*
 * s390x architecture support
 *
 * Register layout for s390x:
 * - r0-r15: General purpose registers
 * - r2: syscall return value and first argument
 * - r2-r7: syscall arguments (arg0-arg5)
 * - r1: syscall number
 * - psw.addr: Program Status Word address (instruction pointer)
 *
 * Note: s390x uses PTRACE_PEEKUSER/PTRACE_POKEUSER for register access
 * instead of PTRACE_GETREGSET/PTRACE_SETREGSET
 */

#define ARCH_HAVE_PEEKUSER_REGS

static struct ptrace_personality arch_personality[1] = {
    {
        offsetof(struct user_regs_struct, gprs[2]),  /* syscall_rv */
        offsetof(struct user_regs_struct, gprs[2]),  /* syscall_arg0 */
        offsetof(struct user_regs_struct, gprs[3]),  /* syscall_arg1 */
        offsetof(struct user_regs_struct, gprs[4]),  /* syscall_arg2 */
        offsetof(struct user_regs_struct, gprs[5]),  /* syscall_arg3 */
        offsetof(struct user_regs_struct, gprs[6]),  /* syscall_arg4 */
        offsetof(struct user_regs_struct, gprs[7]),  /* syscall_arg5 */
        offsetof(struct user_regs_struct, psw.addr), /* reg_ip */
    }
};

static const unsigned long syscall_reg_offset = offsetof(struct user_regs_struct, gprs[1]);

static inline void arch_fixup_regs(struct ptrace_child *child) {
    /* s390x uses a 2-byte instruction for syscall (svc 0), so we need to back up by 2 bytes */
    child->regs.psw.addr -= 2;
}

static inline int arch_set_syscall(struct ptrace_child *child,
                                   unsigned long sysno) {
    return ptrace_command(child, PTRACE_POKEUSER, syscall_reg_offset, sysno);
}

static inline int arch_save_syscall(struct ptrace_child *child) {
    child->saved_syscall = *ptr(&child->regs, syscall_reg_offset);
    return 0;
}

static inline int arch_restore_syscall(struct ptrace_child *child) {
    return arch_set_syscall(child, child->saved_syscall);
}
