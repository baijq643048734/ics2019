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

void free_wp(WP *wp){
	WP *p=head;
	if(wp == head){
		p = head ->next;
		head -> next = free_;
		free_ = head;
		head = p;
	}
	else{
		WP *p1,*p2;
		for(p1=head,p2=head->next;p2!=wp;p1=p1->next,p2=p2->next);
		p1 -> next = wp -> next;
		wp -> next = free_;
		free_ = wp;
	}
}

/* TODO: Implement the functionality of watchpoint */



