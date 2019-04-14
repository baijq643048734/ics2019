#include "monitor/watchpoint.h"
#include "monitor/expr.h"

#define NR_WP 32

static WP wp_pool[NR_WP];
static WP *head, *free_;

void init_wp_pool() {
  int i;
  for (i = 0; i < NR_WP; i ++) {
    wp_pool[i].NO = i;
    wp_pool[i].next = &wp_pool[i + 1];
  }
  wp_pool[NR_WP - 1].next = NULL;

  head = NULL;
  free_ = wp_pool;
}

WP* new_wp(){
	if(free_ == NULL){
		Log("Watchpoint Full!\n");
		assert(0);
	}
	int Num = free_ -> NO;
	free_  = free_ -> next;
	wp_pool[Num].next = head;
	head = &wp_pool[Num];
	return head;
}

//void free_wp(WP *wp){
//	WP *p= 
/* TODO: Implement the functionality of watchpoint */



