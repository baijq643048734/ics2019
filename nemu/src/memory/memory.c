#include "nemu.h"
#include "device/mmio.h"
#define PMEM_SIZE (128 * 1024 * 1024)

#define pmem_rw(addr, type) *(type *)({\
    Assert(addr < PMEM_SIZE, "physical address(0x%08x) is out of bound", addr); \
    guest_to_host(addr); \
    })

uint8_t pmem[PMEM_SIZE];

/* Memory accessing interfaces */

uint32_t paddr_read(paddr_t addr, int len) {
	int mmio_id;
	if((mmio_id = is_mmio(addr))!=-1) return mmio_read(addr,len,mmio_id);
	else
  return pmem_rw(addr, uint32_t) & (~0u >> ((4 - len) << 3));
}

void paddr_write(paddr_t addr, int len, uint32_t data) {
	int mmio_id;
	if((mmio_id=is_mmio(addr))!=-1) mmio_write(addr,len,data,mmio_id);
	else
  memcpy(guest_to_host(addr), &data, len);
}

paddr_t page_translate(vaddr_t addr,bool is_write);
#define IF_CROSS_PAGE(addr,len) ((((addr)+(len)-1) & ~PAGE_MASK)!=((addr)& ~PAGE_MASK))

uint32_t vaddr_read(vaddr_t addr, int len) {
//	if(cpu.cr0.paging){
//		if(IF_CROSS_PAGE(addr,len)){
//			assert(0);
//		}
//		else{
//			paddr_t paddr = page_translate(addr,false);
//			return paddr_read(paddr,len);
//		}
//	}
//	else
//		return paddr_read(addr,len);
		paddr_t paddr = addr;
		if(cpu.cr0.paging) paddr = page_translate(addr,false);
		return paddr_read(paddr,len);
}

void vaddr_write(vaddr_t addr, int len, uint32_t data) {
//	if(cpu.cr0.paging){
//		if(IF_CROSS_PAGE(addr,len)){
//			assert(0);
//		}
//		else{
//			paddr_t paddr = page_translate(addr,true);
//			paddr_write(paddr,len,data);
//		}
//	}
//	else
//		paddr_write(addr, len, data);
	paddr_t paddr = addr;
	if(cpu.cr0.paging) paddr = page_translate(addr,true);
	paddr_write(paddr,len,data);
}

paddr_t page_translate(vaddr_t addr,bool is_write){
	PDE pde;
	PTE pte;
	if(cpu.cr0.protect_enable && cpu.cr0.paging){
		pde.val = paddr_read((intptr_t)((cpu.cr3.page_directory_base<<12) | ((addr >> 22 ) & 0x3ff)<<2),4);
	//	assert(pde.present);
		pde.accessed = 1;
		pte.val = paddr_read((intptr_t)((pde.page_frame<<12) | ((addr>>12) &0x3ff)<<2),4);
	//	assert(pte.present);
		pte.accessed = 1;
		pte.dirty = is_write ? 1 : pte.dirty;
		addr = (pte.page_frame << 12) | (addr & PAGE_MASK);
//		Log("Addr : 0x%08x",addr);
	}
	return addr;
}
