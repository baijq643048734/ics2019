#include "monitor/monitor.h"
#include "monitor/expr.h"
#include "monitor/watchpoint.h"
#include "nemu.h"

#include <stdlib.h>
#include <readline/readline.h>
#include <readline/history.h>

void cpu_exec(uint64_t);

/* We use the `readline' library to provide more flexibility to read from stdin. */
char* rl_gets() {
  static char *line_read = NULL;

  if (line_read) {
    free(line_read);
    line_read = NULL;
  }

  line_read = readline("(nemu) ");

  if (line_read && *line_read) {
    add_history(line_read);
  }

  return line_read;
}

static int cmd_c(char *args) {
  cpu_exec(-1);
  return 0;
}

static int cmd_q(char *args) {
  return -1;
}

static int cmd_help(char *args);
static int cmd_si(char *args);
static int cmd_info(char *args);
static int cmd_x(char *args);
static int cmd_p(char *args);
static int cmd_w(char *args);
static int cmd_d(char *args);

static struct {
  char *name;
  char *description;
  int (*handler) (char *);
} cmd_table [] = {
  { "help", "Display informations about all supported commands", cmd_help },
  { "c", "Continue the execution of the program", cmd_c },
  { "q", "Exit NEMU", cmd_q },
  { "si","Perform several steps", cmd_si},
  { "info","Print Register", cmd_info},
  { "x","Scan memory", cmd_x},
  { "p","Expression evaluation", cmd_p},
  { "w","Set a watchpoint", cmd_w},
  { "d","Delete a watchpoint", cmd_d},

  /* TODO: Add more commands */

};

#define NR_CMD (sizeof(cmd_table) / sizeof(cmd_table[0]))

static int cmd_help(char *args) {
  /* extract the first argument */
  char *arg = strtok(NULL, " ");
  int i;

  if (arg == NULL) {
    /* no argument given */
    for (i = 0; i < NR_CMD; i ++) {
      printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
    }
  }
  else {
    for (i = 0; i < NR_CMD; i ++) {
      if (strcmp(arg, cmd_table[i].name) == 0) {
        printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
        return 0;
      }
    }
    printf("Unknown command '%s'\n", arg);
  }
  return 0;
}

static int cmd_si(char *args){
	char *arg=strtok(NULL," ");
	int n=0;
	if(arg==NULL) arg="0";
	sscanf(arg,"%d",&n);
	if(n==0) cpu_exec(1);
	if(n<0) cpu_exec(-1);
	if(n>0)	cpu_exec(n);
	return 1;
}

static int cmd_info(char *args){
	char *arg=strtok(NULL," ");
	if(strcmp(arg,"r")==0){
		int i,j;
		for(i=0;i<8;i++){
			printf("%s %x\n",regsl[i],cpu.gpr[i]._32);
		}
		for(i=0;i<8;i++){
			printf("%s %x\n",regsw[i],cpu.gpr[i]._16);
		}
		for(i=0;i<8;i++){
			for(j=0;j<2;j++){
				printf("%s %x\n",regsb[i],cpu.gpr[i]._8[j]);
			}
		}
	}
	else if(strcmp(arg,"w")==0){
		list_watchpoint();
	}
	return 1;
}

static int cmd_x(char *args){
	char *arg_1=strtok(NULL," ");
	char *arg_2=strtok(NULL," ");
	int i;
	int len;
	vaddr_t addr,addr_1;
	int hex;
	sscanf(arg_1,"%d",&len);
	if(arg_2[0]=='0') sscanf(arg_2,"%x",&addr);
	else if(arg_2[0]=='$'){
		bool success;
		int result=expr(arg_2,&success);
		addr = result;
	}
	printf("Address    Dword block ... Byte sequence\n");
	for(i=0;i<len;i++){
		addr_1=vaddr_read(addr,4);
		printf("%#010x  ",addr);
		printf("%#010x ",addr_1);
		printf("... ");
		hex=(addr_1&0x000000FF);
		printf("%02x ",hex);
		hex=(addr_1&0x0000FF00) >> 8;
		printf("%02x ",hex);
		hex=(addr_1&0x00FF0000) >> 16;
		printf("%02x ",hex);
		hex=(addr_1&0xFF000000) >> 24;
		printf("%02x ",hex);
		printf("\n");
		addr+=4;
	}
	return 1;
}

static int cmd_p(char *args){
	bool success;
	int result = expr(args , &success);
	if(success) printf("0x%08x %d\n",result,result);
	return 1;
}

static int cmd_w(char *args){
	return set_watchpoint(args);
}

static int cmd_d(char *args){
	int num;
	sscanf(args,"%d",&num);
	return delete_watchpoint(num);
}

void ui_mainloop(int is_batch_mode) {
  if (is_batch_mode) {
    cmd_c(NULL);
    return;
  }

  while (1) {
    char *str = rl_gets();
    char *str_end = str + strlen(str);

    /* extract the first token as the command */
    char *cmd = strtok(str, " ");
    if (cmd == NULL) { continue; }

    /* treat the remaining string as the arguments,
     * which may need further parsing
     */
    char *args = cmd + strlen(cmd) + 1;
    if (args >= str_end) {
      args = NULL;
    }

#ifdef HAS_IOE
    extern void sdl_clear_event_queue(void);
    sdl_clear_event_queue();
#endif

    int i;
    for (i = 0; i < NR_CMD; i ++) {
      if (strcmp(cmd, cmd_table[i].name) == 0) {
        if (cmd_table[i].handler(args) < 0) { return; }
        break;
      }
    }

    if (i == NR_CMD) { printf("Unknown command '%s'\n", cmd); }
  }
}
