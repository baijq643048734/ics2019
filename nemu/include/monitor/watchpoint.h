#ifndef __WATCHPOINT_H__
#define __WATCHPOINT_H__

#include "common.h"

#define NR_WP 32

typedef struct watchpoint {
  int NO;
  struct watchpoint *next;
  char expr[128];
  uint32_t new_val,old_val;

  /* TODO: Add more members if necessary */


} WP;

WP* new_wp();
void free_wp(WP *wp);
static WP wp_pool[NR_WP];
#endif
