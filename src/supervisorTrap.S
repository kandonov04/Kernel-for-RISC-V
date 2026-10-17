# void Riscv::supervisorTrap()

# Interrupt handler - the only place where execution transitions from thread code to kernel code and back
# The entire register file x0..x31 is saved, not just the registers that a called subroutine would be required to save according to the calling convention
# The reason is the asynchronous interrupt: it can arrive at a moment when values in temporary registers may still be live, and kernel code would overwrite them

# A 256-byte block is created on the stack of the thread that was interrupted
# If a context switch occurs during handling, sp points to another thread's stack, so its registers are restored
# x10 (a0) is also restored, so the system call's return value is written to that location

.extern _ZN5Riscv20handleSupervisorTrapEv

.align 4
.global _ZN5Riscv14supervisorTrapEv
.type _ZN5Riscv14supervisorTrapEv, @function
_ZN5Riscv14supervisorTrapEv:
    addi sp, sp, -256
    .irp index 0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31
    sd x\index, \index * 8(sp)
    .endr

    call _ZN5Riscv20handleSupervisorTrapEv

    .irp index 0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31
    ld x\index, \index * 8(sp)
    .endr
    addi sp, sp, 256

    sret