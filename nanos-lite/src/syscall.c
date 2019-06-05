#include "common.h"
#include "syscall.h"
#include "fs.h"

extern ssize_t fs_write(int fd,const void *buf,size_t len);
int mm_brk(uint32_t new_brk);

static inline uintptr_t sys_open(uintptr_t pathname, uintptr_t flags, uintptr_t mode) {
  TODO();
  return 1;
}

static inline uintptr_t sys_write(uintptr_t fd, uintptr_t buf, uintptr_t len) {
	uintptr_t i = -1;
	if(fd == 1 || fd == 2){
		i++;
		for(;len>0;len--){
			_putc(((char*)buf)[i]);
			i++;
		}
	}
	Log("1");
  return i;
}

static inline uintptr_t sys_read(uintptr_t fd, uintptr_t buf, uintptr_t len) {
  TODO();
  return 1;
}

static inline uintptr_t sys_lseek(uintptr_t fd, uintptr_t offset, uintptr_t whence) {
  return fs_lseek(fd, offset, whence);
}

static inline uintptr_t sys_close(uintptr_t fd) {
  TODO();
  return 1;
}

static inline uintptr_t sys_brk(uintptr_t new_brk) {
  return mm_brk((uint32_t)new_brk);
}


_RegSet* do_syscall(_RegSet *r) {
  uintptr_t a[4];
  a[0] = SYSCALL_ARG1(r);

  switch (a[0]) {
	  case SYS_none:
		  SYSCALL_ARG1(r)=1;
		  break;
	  case SYS_exit:
		  _halt(SYSCALL_ARG2(r));
		  break;
	  case SYS_write:
		  SYSCALL_ARG1(r) = sys_write(SYSCALL_ARG2(r),SYSCALL_ARG3(r),SYSCALL_ARG4(r));
		  break;
	  case SYS_brk:
		  Log("2");
		  SYSCALL_ARG1(r) = sys_brk(SYSCALL_ARG2(r));
		  break;
    default: panic("Unhandled syscall ID = %d", a[0]);
  }

  return NULL;
}
