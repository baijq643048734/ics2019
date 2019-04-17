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

void free_wp(int num){
	WP *p;
	if(num == head -> NO){
		p = head ->next;
		head -> next = free_;
		free_ = head;
		head = p;
	}
	else{
		WP *p1,*p2,*wp;
		for(p1=head,p2=head->next;p2->NO!=num;p1=p1->next,p2=p2->next);
		wp = &wp_pool[num];
		p1 -> next = wp -> next;
		wp -> next = free_;
		free_ = wp;
	}
}

int set_watchpoint(char *e){
	bool success;
	int result = expr(e,&success);
	if(success == false){
		printf("Expr is wrong!\n");
	}
	WP *p = new_wp();
	printf("Set watchpoint #%d\n",p -> NO);
	strcpy(p -> expr,e);
	printf("expr = %s\n", p -> expr);
	p -> old_val = result;
	printf("old value = 0x%08x\n",p -> old_val);
	return 1;
}

bool delete_watchpoint(int NO){
	if(head == NULL){
		printf("There is no watchpoint!\n");
		return false;
	}
	free_wp(NO);
	printf("Watchpoint %d deleted\n", NO);
	return true;
}

void list_watchpoint(void){
	if(head == NULL){
		printf("There is no watchpoint in pool!\n");
		assert(0);
	}
	printf("NO Old Value  Expr\n");
	WP *p=head;
	for(;p!=NULL;p=p->next){
		printf("%d  ",p->NO);
		printf("0x%08x ",p->old_val);
		printf("%s\n",p->expr);
	}
}

WP* scan_watchpoint(void){
	bool success;
	WP *p=head;
	int result;
	for(;p!=NULL;p=p->next){
		result = expr(p -> expr,&success);
		if(result!= p -> old_val){
			p->new_val = result;
			printf("Hit watchpoint %d at address 0x%08x\n",p->NO,p->old_val);
			return p;
		}
	}
	return NULL;
}

/* TODO: Implement the functionality of watchpoint */



