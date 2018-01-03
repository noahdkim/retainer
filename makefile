prompt.o: prompt.c 
	-std=c99 -Wall prompt.c -ledit -o prompt
