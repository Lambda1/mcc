#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[])
{
	if (argc != 2)
	{
	  fprintf(stderr, "This program requires more than two arguments.\n");
	  return 1;
  }

	printf(".intel_syntax noprefix\n");
	printf(".global main\n");

	printf("main:\n");

	// CODE 
	char* ptr = argv[1];
	printf("	mov rax, %ld\n", strtol(ptr, &ptr, 10));
	while (*ptr)
	{
	  if (*ptr == '+')
	  {
			++ptr;
			printf("	add rax, %ld\n", strtol(ptr, &ptr, 10));
			continue;
		}

	  if (*ptr == '-')
	  {
			++ptr;
			printf("	sub rax, %ld\n", strtol(ptr, &ptr, 10));
			continue;
		}
	
	  fprintf(stderr, "error: %c\n", *ptr);
	  return 1;
  }

	printf("	ret\n");

	return 0;
}
