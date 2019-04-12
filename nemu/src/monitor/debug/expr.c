#include "nemu.h"

/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <string.h>
#include <sys/types.h>
#include <regex.h>

enum {
  TK_NOTYPE = 256, TK_EQ,
  TK_10_NUM, TK_16_NUM,
  TK_REGISTER, TK_LP,
  TK_RP

  /* TODO: Add more token types */

};

static struct rule {
  char *regex;
  int token_type;
} rules[] = {

  /* TODO: Add more rules.
   * Pay attention to the precedence level of different rules.
   */

  {" +", TK_NOTYPE},									// spaces
  {"\\+", '+'},											// plus
  {"==", TK_EQ},										// equal
  {"[0-9]+", TK_10_NUM},								// decimal number
  {"0x[0-9a-fA-F]+", TK_16_NUM},						// hexadecimal number
  {"\\$e(ax|bx|cx|dx|sp|bp|si|di|ip)", TK_REGISTER},	// register
  {"\\(", TK_LP},										// left parentheses
  {"\\)", TK_RP},										// right parentheses
  {"\\-", '-'},											// minus
  {"\\*", '*'},											// multiply
  {"\\/", '/'},											// divide
};

#define NR_REGEX (sizeof(rules) / sizeof(rules[0]) )

static regex_t re[NR_REGEX];

/* Rules are used for many times.
 * Therefore we compile them only once before any usage.
 */
void init_regex() {
  int i;
  char error_msg[128];
  int ret;

  for (i = 0; i < NR_REGEX; i ++) {
    ret = regcomp(&re[i], rules[i].regex, REG_EXTENDED);
    if (ret != 0) {
      regerror(ret, &re[i], error_msg, 128);
      panic("regex compilation failed: %s\n%s", error_msg, rules[i].regex);
    }
  }
}

typedef struct token {
  int type;
  char str[32];
} Token;

Token tokens[32];
int nr_token;

static bool make_token(char *e) {
  int position = 0;
  int i;
  regmatch_t pmatch;

  nr_token = 0;

  while (e[position] != '\0') {
    /* Try all rules one by one. */
    for (i = 0; i < NR_REGEX; i ++) {
      if (regexec(&re[i], e + position, 1, &pmatch, 0) == 0 && pmatch.rm_so == 0) {
        char *substr_start = e + position;
        int substr_len = pmatch.rm_eo;

        Log("match rules[%d] = \"%s\" at position %d with len %d: %.*s",
            i, rules[i].regex, position, substr_len, substr_len, substr_start);
        position += substr_len;

        /* TODO: Now a new token is recognized with rules[i]. Add codes
         * to record the token in the array `tokens'. For certain types
         * of tokens, some extra actions should be performed.
         */
		int j;
        switch (rules[i].token_type) {
			case TK_NOTYPE:
				break;
			case '+':
			case '-':
			case '*':
			case '/':
			case TK_EQ:
			case TK_LP:
			case TK_RP:
				tokens[nr_token].type = rules[i].token_type;
				nr_token++;
				break;
			case TK_16_NUM:
			case TK_10_NUM:
			case TK_REGISTER:
				for(j=0;j<substr_len;j++){
					tokens[nr_token].str[j] = *(substr_start + j);
				}
				tokens[nr_token].str[j]='\0';
				tokens[nr_token].type = rules[i].token_type;
				nr_token++;
				break;
			default:
				Log("Match failed!\n");
				assert(0);
        }
        break;
      }
    }

    if (i == NR_REGEX) {
      printf("no match at position %d\n%s\n%*.s^\n", position, e, position, "");
      return false;
    }
  }

  return true;
}

bool check_parentheses(int p,int q){
	if(tokens[p].type !=TK_LP || tokens[q].type !=TK_RP)
		return false;
	int sign=0;
	for(;p<q;p++){
		if(tokens[p].type == TK_LP) sign++;
		else if(tokens[p].type == TK_RP) sign--;
		if(sign == 0) return false;
	}
	return true;
}

int find_dominated_op(int p ,int q){
	int level = 0;
	int op = 0;
	int i;
	for(i=p;i<=q;i++){
		if(tokens[i].type == TK_LP){
			level++;
		}
		else if(tokens[i].type == TK_RP){
			level--;
		}
		if(level == 0){
			if(tokens[i].type == TK_EQ) op=i;
			else if(tokens[i].type == '+' || tokens[i].type == '-') op=i;
			else if(tokens[i].type == '*' || tokens[i].type == '/') op=i;
		}
	}
	return op;
}

uint32_t eval(int p,int q){
	uint32_t n,val1,val2;
	if(p > q){
		printf("Bad expression! \n");
		assert(0);
	}
	else if (p==q){
		if(tokens[p].type == TK_16_NUM){
			sscanf(tokens[p].str,"%x",&n);
			return n;
		}
		if(tokens[p].type == TK_10_NUM){
			sscanf(tokens[p].str,"%d",&n);
			return n;
		}
		if(tokens[p].type == TK_REGISTER){
			if(strcmp(tokens[p].str,"&eax")==0) return cpu.eax;
			else if(strcmp(tokens[p].str,"&ebx")==0) return cpu.ebx;
			else if(strcmp(tokens[p].str,"&ecx")==0) return cpu.ecx;
			else if(strcmp(tokens[p].str,"&edx")==0) return cpu.edx;
			else if(strcmp(tokens[p].str,"&esp")==0) return cpu.esp;
			else if(strcmp(tokens[p].str,"&ebp")==0) return cpu.ebp;
			else if(strcmp(tokens[p].str,"&esi")==0) return cpu.esi;
			else if(strcmp(tokens[p].str,"&edi")==0) return cpu.edi;
			else if(strcmp(tokens[p].str,"&eip")==0) return cpu.eip;
			else{
				printf("Register Wrong! \n");
				assert(0);
			}
		}
	}
	else if(check_parentheses(p,q) == true){
		return eval(p+1,q-1);
	}
	else{
		int op = find_dominated_op(p,q);
		val1 = eval(p,op-1);
		val2 = eval(op+1,q);
		switch(tokens[op].type){
			case '+':
				return val1 + val2;
			case '-':
				return val1 - val2;
			case '*':
				return val1 * val2;
			case '/':
				return val1 / val2;
			case TK_EQ:
				return val1 == val2;
			default:
				printf("Operator Wrong!\n");
				assert(0);
		}
	}
	return 0;
}


uint32_t expr(char *e, bool *success) {
  if (!make_token(e)) {
    *success = false;
    return 0;
  }

  /* TODO: Insert codes to evaluate the expression. */
  
  *success =true;
  return eval(0,nr_token-1);
}
