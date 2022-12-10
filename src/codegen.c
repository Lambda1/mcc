#include <ctype.h>

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdarg.h>

#include <string.h>
#include <assert.h>

#include "mcc.h"

void GenLval(const struct Node* const pNode)
{
	if (pNode->kind != ND_LVAR)
	{
		fprintf(stderr, "Left is not varialble.\n");
		exit(1);
	}

	printf("  mov rax, rbp\n");
	printf("  sub rax, %d\n", pNode->offset);
	printf("  push rax\n");
}

// スタックマシン
void Gen(const struct Node* const pNode)
{
	assert(pNode != NULL);

	static int jumpIndex = 0;

	switch(pNode->kind)
	{
		case ND_RTN:
			Gen(pNode->pLhs);
			printf("  pop rax\n");
			printf("  mov rsp, rbp\n");
			printf("  pop rbp\n");
			printf("  ret\n");
			return;
		case ND_IF: // if(A) B else C
		{
			int cnt = jumpIndex++;
			Gen(pNode->pCond); // A
			printf("  pop rax\n");
			printf("  cmp rax, 0\n");
			if (pNode->pElse == NULL) // if文単体
			{
				printf("  je .Lend%d\n", cnt);
				Gen(pNode->pThen); // B
			}
			else // if-else
			{
				printf("  je .Lelse%d\n", cnt);
				Gen(pNode->pThen); // B
				printf("  jmp .Lend%d\n", cnt);
				printf(".Lelse%d:\n", cnt);
				Gen(pNode->pElse); // C
			}
			printf(".Lend%d:\n", cnt);
			return;
		}
		case ND_WHILE:
		{
			int cnt = jumpIndex++;
			printf(".Lbegin%d:\n", cnt);
			Gen(pNode->pCond);
			printf("  pop rax\n");
			printf("  cmp rax, 0\n");
			printf("  je .Lend%d\n", cnt);
			Gen(pNode->pThen);
			printf("  jmp .Lbegin%d\n", cnt);
			printf(".Lend%d:\n", cnt);
			return;
		}
		case ND_BLOCK:
			for (struct Node* pTmp = pNode->pBlock; pTmp != NULL; pTmp = pTmp->pBlock)
			{
				if (pTmp->isReadBlock == 0)
				{
					// 読み込んだブロックとして登録して，読み込み済みはスキップする
					pTmp->isReadBlock = 1;
					Gen(pTmp);
				}
			}
			return;
		case ND_FUNC:
		{
			assert(pNode->pLabel != NULL);
			char tmp[100] = {};
			assert(sizeof(tmp)/sizeof(tmp[0]) > pNode->labelLen);
			strncpy(tmp, pNode->pLabel, pNode->labelLen);
			tmp[pNode->labelLen] = '\0';
			printf("  call %s\n", tmp);
		}
			return;
	}

	switch (pNode->kind)
	{
		case ND_NUM:
			printf("  push %d\n", pNode->value);
			return;
		case ND_LVAR:
			GenLval(pNode);
			printf("  pop rax\n");
			printf("  mov rax, [rax]\n");
			printf("  push rax\n");
			return;
		case ND_ASSIGN:
			GenLval(pNode->pLhs);
			Gen(pNode->pRhs);

			printf("  pop rdi\n");
			printf("  pop rax\n");
			printf("  mov [rax], rdi\n");
			printf("  push rdi\n");
			return;
	}

	Gen(pNode->pLhs);
	Gen(pNode->pRhs);

	printf("  pop rdi\n");
	printf("  pop rax\n");

	switch(pNode->kind)
	{
		case ND_ADD:
			printf("  add rax, rdi\n");
			break;
		case ND_SUB:
			printf("  sub rax, rdi\n");
			break;
		case ND_MUL:
			printf("  imul rax, rdi\n");
			break;
		case ND_DIV:
			printf("  cqo\n");
			printf("  idiv rdi\n");
			break;
		case ND_EQU:
			printf("  cmp rax, rdi\n");
			printf("  sete al\n");
			printf("  movzb rax, al\n");
			break;
		case ND_NEQ:
			printf("  cmp rax, rdi\n");
			printf("  setne al\n");
			printf("  movzb rax, al\n");
			break;
		case ND_LTH:
			printf("  cmp rax, rdi\n");
			printf("  setl al\n");
			printf("  movzb rax, al\n");
			break;
		case ND_LEQ:
			printf("  cmp rax, rdi\n");
			printf("  setle al\n");
			printf("  movzb rax, al\n");
			break;
		default:
			fprintf(stderr, "This kind is not recognized.");
			exit(1);
	}

	printf("  push rax\n");
}

