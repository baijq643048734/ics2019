#include "cpu/exec.h"
#include "memory/mmu.h"

void raise_intr(uint8_t NO, vaddr_t ret_addr) {
  /* TODO: Trigger an interrupt/exception with ``NO''.
   * That is, use ``NO'' to index the IDT.
   */
  uint32_t idtr_base = cpu.idtr.base;
  uint32_t eip_lo,eip_hi,offset;

  eip_lo = vaddr_read(idtr_base + NO * 8,4) & 0x0000ffff;
  eip_hi = vaddr_read(idtr_base + NO * 8+4,4) & 0xffff0000;

  offset = eip_lo | eip_hi;
  
  rtl_push(&cpu.eflags.EFLAGS);
  rtl_push((rtlreg_t *)&cpu.cs);
  rtl_push(&ret_addr);

  decoding.jmp_eip = offset;
  decoding.is_jmp = 1;
}

void dev_raise_intr() {
}
