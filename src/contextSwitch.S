# void TCB::contextSwitch(Context* oldContext, Context* runningContext)

# Context is a structure with two fields: { uint64 ra; uint64 sp; }
#   a0 - &old -> context (context of the current thread)
#   a1 - &running -> context (context of the thread being given the processor)

# Registers s0 through s11 are saved on the thread's own stack, just as the interrupt handler saves the register file

# The interrupt handler saves registers at the user mode - kernel boundary.
# Here, the transition is from kernel to kernel, when kernel code already holds its own values in registers s0 through s11
# By calling convention, these are saved by the called subroutine, which no longer applies when control is handed over to another thread

# The other registers are not saved:
#   x1 (ra) and x2 (sp) are part of the Context structure
#   x0 is always zero
#   x3 (gp) and x4 (tp) are global to the entire system and are set by the startup code
#   t0 - t6 and a0 - a7 are caller-saved, so they never hold values that need to survive a function call

# The ret instruction jumps to the newly loaded ra, so execution continues in the context of the new thread

.global _ZN3TCB13contextSwitchEPNS_7ContextES1_
.type _ZN3TCB13contextSwitchEPNS_7ContextES1_, @function
_ZN3TCB13contextSwitchEPNS_7ContextES1_:
    addi sp, sp, -96
    .irp index, 0,1,2,3,4,5,6,7,8,9,10,11
    sd s\index, \index * 8(sp)
    .endr

    sd ra, 0 * 8(a0)
    sd sp, 1 * 8(a0)

    ld ra, 0 * 8(a1)
    ld sp, 1 * 8(a1)

    .irp index, 0,1,2,3,4,5,6,7,8,9,10,11
    ld s\index, \index * 8(sp)
    .endr
    addi sp, sp, 96

    ret