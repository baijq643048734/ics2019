#include "cpu/exec.h"
#include "memory/mmu.h"

void raise_intr(uint8_t NO, vaddr_t ret_addr) {
  /* TODO: Trigger an interrupt/exception with ``NO''.
   * That is, use ``NO'' to index the IDT.
   */

  union{
	  GateDesc gd;
	  struct{
		  uint32_t low,high;
	  };
  }item;
  vaddr_t addr = cpu.idtr.base + NO*8;
  item.low = vaddr_read(addr,4);
  item.high = vaddr_read(addr+4,4);
  
  rtl_push(&cpu.eflags.EFLAGS);
  rtl_push((rtlreg_t *)&cpu.cs);
  rtl_push(&ret_addr);
  cpu.eflags.IF=0;

  decoding.jmp_eip = (item.gd.offset_15_0 & 0xFFFF) | ((item.gd.offset_31_16 & 0xFFFF) << 16);
  decoding.is_jmp = 1;
}

void dev_raise_intr() {
}
