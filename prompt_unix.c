#include <stdio.h>
#include <stdlib.h>

#include <editline/readline.h>
#include <editline/history.h>
/* Declare a buffer for user input of size 2048  */

int main(int argc, char** argv){
	puts("Lispy version 0.0.0.0.1");
	puts("Press Ctrl+C to Exit\n");

	/* In never ending loop */
	while (1) {
		char* input = readline("lispy> ");
		
		/* Add input to history */
		add_history(input);

		printf("No you're a %s\n", input);
		
		free(input);
	}

	return 0;
}
