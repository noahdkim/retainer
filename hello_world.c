#include <stdio.h>
int add_together(int a, int b);
int main(int argc, char** argv){
  puts("Hello World");
  printf("%d", add_together(1, 2));
  return 0;
} 

int add_together(int a, int b){
    return a+b;
}
