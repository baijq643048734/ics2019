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
  TK_RP, TK_NOT_EQ,
  TK_AND, TK_OR,
  TK_DEREF, TK_NOT

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
  {"0[xX][0-9a-fA-F]+", TK_16_NUM},						// hexadecimal number
  {"[0-9]+", TK_10_NUM},								// decimal number
  {"\\$e(ax|bx|cx|dx|sp|bp|si|di|ip)", TK_REGISTER},	// register
  {"\\(", TK_LP},										// left parentheses
  {"\\)", TK_RP},										// right parentheses
  {"\\-", '-'},											// minus
  {"\\*", '*'},											// multiply & dereference
  {"\\/", '/'},											// divide
  {"!=", TK_NOT_EQ},									// not equal
  {"&&", TK_AND},										// logical and
  {"\\|\\|", TK_OR},									// logical or
  {"!", TK_NOT},										// not
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
			case TK_NOT_EQ:
			case TK_AND:
			case TK_OR:
			case TK_NOT:
			case TK_DEREF:
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
	int P_level = 0;
	int TK_AND_L = 1;
	int TK_OR_L = 1;
	int TK_EQ_L = 2;
	int TK_NOT_EQ_L = 2;
	int TK_PLUS_L = 3;
	int TK_MINUS_L = 3;
	int TK_MULTI_L = 4;
	int TK_DIV_L = 4;
	int TK_NOT_L = 5;
	int TK_DEREF_L = 5;
	int op = 0;
	int opsign = 65535;
	int i;
	for(i=p;i<=q;i++){
		if(tokens[i].type == TK_LP){
			P_level++;
		}
		else if(tokens[i].type == TK_RP){
			P_level--;
		}
		if(P_level == 0){
			if(tokens[i].type == TK_EQ && opsign > TK_EQ_L){
			   	op = i;
				opsign = TK_EQ_L;
			}
			else if(tokens[i].type == '+' && opsign > TK_PLUS_L){
			   	op=i;
				opsign = TK_PLUS_L;
			}
			else if(tokens[i].type == '-' && opsign > TK_MINUS_L){
				op=i;
				opsign = TK_MINUS_L;
			}
			else if(tokens[i].type == '*' && opsign > TK_MULTI_L){
			   	op=i;
				opsign = TK_MULTI_L;
			}
			else if(tokens[i].type == '/' && opsign > TK_DIV_L){
				op=i;
				opsign = TK_DIV_L;
			}
			else if(tokens[i].type == TK_NOT_EQ && opsign > TK_NOT_EQ_L){
				op=i;
				opsign = TK_NOT_EQ_L;
			}
			else if(tokens[i].type == TK_AND && opsign > TK_AND_L){
				op=i;
				opsign = TK_AND_L;
			}
			else if(tokens[i].type == TK_OR && opsign > TK_OR_L){
				op=i;
				opsign = TK_OR_L;
			}
			else if(tokens[i].type == TK_NOT && opsign > TK_NOT_L){
				op=i;
				opsign = TK_NOT_L;
			}
			else if(tokens[i].type == TK_DEREF && opsign > TK_DEREF_L){
				op=i;
				opsign = TK_DEREF_L;
			}
		}
	}
	return op;
}

uint32_t eval(int p,int q){
	uint32_t n;
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
			if(strcmp(tokens[p].str,"$eax")==0) return cpu.eax;
			else if(strcmp(tokens[p].str,"$ebx")==0) return cpu.ebx;
			else if(strcmp(tokens[p].str,"$ecx")==0) return cpu.ecx;
			else if(strcmp(tokens[p].str,"$edx")==0) return cpu.edx;
			else if(strcmp(tokens[p].str,"$esp")==0) return cpu.esp;
			else if(strcmp(tokens[p].str,"$ebp")==0) return cpu.ebp;
			else if(strcmp(tokens[p].str,"$esi")==0) return cpu.esi;
			else if(strcmp(tokens[p].str,"$edi")==0) return cpu.edi;
			else if(strcmp(tokens[p].str,"$eip")==0) return cpu.eip;
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
		int val1=0;
		int val2=0;
		if(p < op){
			val1 = eval(p,op-1);
		}
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
			case TK_NOT_EQ:
				return val1 != val2;
			case TK_AND:
				return val1 && val2;
			case TK_OR:
				return val1 || val2;
			case TK_NOT:
				return !val2;
			case TK_DEREF:
				return vaddr_read(val2,4);
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
  
  for(int i = 0; i<nr_token;i++){
	  if(tokens[i].type == '*' && (i==0 || (tokens[i-1].type != TK_16_NUM && tokens[i-1].type != TK_10_NUM && tokens[i-1].type != TK_RP)))
		  tokens[i].type = TK_DEREF;
  }
  *success = true;
  return eval(0,nr_token-1);
}

