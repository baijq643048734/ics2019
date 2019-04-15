#ifndef __WATCHPOINT_H__
#define __WATCHPOINT_H__

#include "common.h"

typedef struct watchpoint {
  int NO;
  struct watchpoint *next;
  char expr[128];
  uint32_t new_val,old_val;

  /* TODO: Add more members if necessary */


} WP;

WP* new_wp();
void free_wp(int num);
#endif
