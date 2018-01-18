#include "mpc.h"

/* If we are compiling on Windows compile these functions */
#ifdef _WIN32
#include <string.h>

static char buffer[2048];

/* Fake readline function */
char* readline(char* prompt) {
	  fputs(prompt, stdout);
	    fgets(buffer, 2048, stdin);
	      char* cpy = malloc(strlen(buffer)+1);
	        strcpy(cpy, buffer);
		  cpy[strlen(cpy)-1] = '\0';
		    return cpy;
}

/* Fake add_history function */
void add_history(char* unused) {}

/* Otherwise include the editline headers */
#else
#include <editline/readline.h>
#include <editline/history.h>
#endif

typedef struct {
	int type;
	long num;
	/* Error and Symbol types have string data */
	char* err;
	char* sym;
	/* Count and Pointer to a list of "lval" */
	int count;
	struct lval** cell;
	
} lval;

enum { LVAL_NUM, LVAL_ERR, LVAL_SYM, LVAL_SEXPR };

enum { LERR_DIV_ZERO, LERR_BAD_OP, LERR_BAD_NUM };

lval* lval_num(long x) {
	lval* v = malloc(sizeof(lval));
	v->type = LVAL_NUM;
	v->num = x;
	return v;
}

lval* lval_err(char* m) {
	lval* v = malloc(sizeof(lval));
	v->type = LVAL_ERR;
	v->err = malloc(strlen(m) + 1);
	strcpy(v->err, m);
	return v;
}

lval* lval_sym(char* s){
	lval* v = malloc(sizeof(lval));
	v->type = LVAL_SYM;
	v->sym = malloc(strlen(s) + 1);
	strcpy(v->sym, s);
	return v;
}

lval* lval_sexpr(void){
	lval* v =  malloc(sizeof(lval));
	v->type = LVAL_SEXPR;
	v->count = 0;
	v->cell = NULL;
	return v;
}

void lval_del(lval* v){
	switch(v->type){
		/* num doesn't do extra malloc */
		case LVAL_NUM: break;
	
		/* err or sym must free string data */	       
		case LVAL_ERR: free(v->err); break;
		case LVAL_SYM: free(v->sym); break;

		/* Sexpr must delete all items inside */
		case LVAL_SEXPR:
			for (int i = 0; i < v->count; i++){
				lval_del(v->cell[i]);
			}
			free(v->cell);
			break;
	}
}

lval* lval_read_num(mpc_ast_t* t){
	errno = 0;
	long x = strtol(t->contents, NULL, 10);
	return errno != ERANGE ?
		lval_num(x) : lval_err("invalid number");
}

lval* lval_add(lval* v, lval* x){
	v->count++;
	v->cell = realloc(v->cell, sizeof(lval*) * v->count);
	v->cell[v->count-1] = x;
	return v;
}

lval* lval_read(mpc_ast_t* t){
	/* Return conversion of num to num and sym to sym */
	if (strstr(t->tag, "number")) { return lval_read_num(t); }
	if (strstr(t->tag, "symbol")) { return lval_sym(t->contents); }

	lval* x = NULL;
	if (strcmp(t->tag, ">") == 0) { x = lval_sexpr(); }
	if (strstr(t->tag, "sexpr")) { x = lval_sexpr(); }
	
	for (int i = 0; i < t->children_num; i++) {
		if (strcmp(t->children[i]->contents, "(") == 0) { continue;}
		if (strcmp(t->children[i]->contents, ")") == 0) { continue;}
		if (strcmp(t->children[i]->contents, "regex" == 0)) { continue; }
		x = lval_add(x, lval_read(t->children[i]));
	}
	return x;
}


void lval_print(lval *v);

void lval_expr_print(lval* v, char open, char close) {
	  putchar(open);
	    for (int i = 0; i < v->count; i++) {

		        /* Print Value contained within */
		        lval_print(v->cell[i]);

			    /* Don't print trailing space if last element */
			    if (i != (v->count-1)) {
				          putchar(' ');
					      }
			      }
	      putchar(close);
}

void lval_print(lval* v){
	switch (v->type) {
		case LVAL_NUM:   printf("%li", v->num); break;
		case LVAL_ERR:   printf("Error: %s", v->err); break;
		case LVAL_SYM:   printf("%s", v->sym); break;
		case LVAL_SEXPR: lval_expr_print(v, '(', ')'); break;
	}									       
}

void lval_println(lval* v) { lval_print(v); putchar('\n');}






int main(int argc, char** argv) {
	/* Create Some Parsers */
	mpc_parser_t* Number   = mpc_new("number");
	mpc_parser_t* Symbol   = mpc_new("symbol");
	mpc_parser_t* Sexpr    = mpc_new("sexpr");
	mpc_parser_t* Expr     = mpc_new("expr");
	mpc_parser_t* Lispy    = mpc_new("lispy");

	mpca_lang(MPCA_LANG_DEFAULT,
			"								\
				number   : /-?[0-9]+/;					\
				symbol   : '+' | '-' | '/' | '*' | \"add\" | \"min\";   \
				sexpr    : '(' <expr>* ')';         			\
				expr     : <number> | <symbol> | <sexpr>;       	\
			        lispy	 : /^/ <expr>+ /$/;			\
			",
	Number, Symbol, Sexpr, Expr, Lispy);
		
		
	puts("Lispy Version 0.0.0.0.1");
	puts("Press Ctrl+c to Exit\n");
	       
	while (1) {
		/* Now in either case readline will be correctly defined */
		char* input = readline("retainer> ");
		add_history(input);

		mpc_result_t r;
		if(mpc_parse("<stdin>", input, Lispy, &r)){
			/* On success eval AST and print result */
			lval* x = lval_read(r.output);
		        lval_println(x);
			lval_del(x);
		} else {
			mpc_err_print(r.error);
			mpc_err_delete(r.error);
		}
  		
		free(input);
 	}
	mpc_cleanup(5, Number, Symbol, Sexpr, Expr, Lispy);
        return 0;
}
