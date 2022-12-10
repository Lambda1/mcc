#include <ctype.h>

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdarg.h>

#include <string.h>
#include <assert.h>

#include "mcc.h"

int main(int argc, char *argv[])
{
	if (argc < 2)
	{
		fprintf(stderr, "This program requires more than two arguments(argc=%d).\n", argc);
		return 1;
	}

	char* const userInput = argv[1];

	struct LocalVar firstLVar;
	firstLVar.next = NULL;
	firstLVar.name = NULL;
	firstLVar.len = 0;
	firstLVar.offset = 0;

	// トークナイズ
	struct Token* pToken = Tokenize(userInput);
	struct Token* pHead = pToken;
	if (argc > 2 && strncmp(argv[2], "token", 5) == 0) { printf("\ntest token\n"); DebugPrintTokens(pHead); }
	struct Node* pCodes[100] = {};
	const int codesSize = sizeof(pCodes)/sizeof(pCodes[0]);
	Program(pCodes, &pToken, userInput, &firstLVar);
	if (argc > 2 && strncmp(argv[2], "node", 4) == 0) { printf("\ntest node\n"); for (int i = 0; i < codesSize; ++i) { DebugPrintNodes(pCodes[i]); } }

	pToken = pHead;

	// アセンブリ前半
	printf(".intel_syntax noprefix\n");
	printf(".global main\n");
	printf("main:\n");

	// プロローグ
	printf("  push rbp\n");
	printf("  mov rbp, rsp\n");
	printf("  sub rsp, %d\n", 8 *26); // 8Bytes * 26個

	// 先頭からコード生成
	for (int i = 0; pCodes[i] != NULL; ++i)
	{
		Gen(pCodes[i]);
		printf("  pop rax\n");
	}

	// エピローグ
	printf("  mov rsp, rbp\n");
	printf("  pop rbp\n");
	printf("  ret\n");

	int tokenMemoryCount = 0, nodeMemoryCount = 0, lvarMemoryCount = 0;
	tokenMemoryCount = ReleaseTokenMemory(pHead);
	for (int i = 0; i < codesSize; ++i) { nodeMemoryCount = ReleaseNodeMemory(pCodes[i]); }
	lvarMemoryCount = ReleaseLocalVarMemory(firstLVar.next);

	if (argc > 2 && strncmp(argv[2], "memory", 6) == 0)
	{
		printf("token: %d\n", tokenMemoryCount);
		printf("node: %d\n", nodeMemoryCount);
		printf("lvar: %d\n", lvarMemoryCount);
	}

	return 0;
}
