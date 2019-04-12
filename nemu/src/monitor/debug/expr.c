#include "nemu.h"

/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
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

uint32_t expr(char *e, bool *success) {
  if (!make_token(e)) {
    *success = false;
    return 0;
  }

  /* TODO: Insert codes to evaluate the expression. */
  TODO();

  return 0;
}
