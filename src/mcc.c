#include <ctype.h>

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdarg.h>

#include <string.h>
#include <assert.h>

// -- DEFINE --
// トークン: 単語
enum TokenKind
{
	TK_NONE,

	TK_RESERVED, // 記号
	TK_NUM,      // 整数トークン
	TK_EOF,      // 入力終了トークン
};

struct Token
{
	enum TokenKind kind;
	struct Token *next;
	int value;
	const char* str;
};

// 抽象構文木ノード
enum NodeKind
{
	ND_NONE,

	ND_ADD,
	ND_SUB,
	ND_MUL,
	ND_DIV,
	ND_NUM,
};

struct Node
{
	enum NodeKind kind;
	struct Node* pLhs;
	struct Node* pRhs;
	int value; // kind == ND_NUM
};

// -- DEBUG --
// トークン構造体表示
void DebugPrintToken(const struct Token* const pToken)
{
	const char array[] = {'X', 'R', 'N', 'E'};
	assert(pToken->kind < (sizeof(array)/sizeof(const char)));
	printf("Token Info: %p\n", pToken);
	printf("enum : %c\n", array[pToken->kind]);
	printf("next : %p\n", pToken->next);
	printf("value: %d\n", pToken->value);
	printf("str  : %c(%d)\n", *(pToken->str), (int)(*pToken->str));
}
void DebugPrintTokens(const struct Token* pToken)
{
	while(pToken != NULL)
	{
		DebugPrintToken(pToken);
		pToken = pToken->next;
	}
}
// ノード構造体表示
void DebugPrintNode(const struct Node* const pNode)
{
	const char array[] = {'X', '+', '-', '*', '/', 'n'};
	assert(pNode->kind < (sizeof(array)/sizeof(const char)));
	printf("Node Info: %p\n", pNode);
	printf("kind : %c\n", array[pNode->kind]);
	printf("pLhs : %p\n", pNode->pLhs);
	printf("pRhs : %p\n", pNode->pRhs);
	printf("value: %d\n", pNode->value);
}
void DebugPrintNodes(const struct Node* const pRootNode)
{
	if (pRootNode == NULL) { return; }
	DebugPrintNodes(pRootNode->pLhs);
	DebugPrintNode(pRootNode);
	DebugPrintNodes(pRootNode->pRhs);
}

// -- FUNCTION --
// エラーでシバく
void ErrorAt(const char* const loc, const char* const userInput, char* fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);

	const int pos = (int)(loc - userInput);
	fprintf(stderr, "%s\n", userInput);
	fprintf(stderr, "%*s", pos, " ");
	fprintf(stderr, "^ ");
	fprintf(stderr, fmt, ap);
	fprintf(stderr, "\n");

	exit(1);
}

// -- TOKEN --
// トークンが期待する記号か判定
bool IsExpectedToken(const char op, const struct Token* const pToken)
{
	assert(pToken != NULL);
	if (pToken->kind != TK_RESERVED || pToken->str[0] != op)
	{
		return false;
	}
	return true;
}
// トークンが期待する整数か判定
bool IsExpectedNumber(const struct Token* const pToken)
{
	assert(pToken != NULL);
	if (pToken->kind != TK_NUM)
	{
		return false;
	}
	return true;
}
// トークンがEOFか判定
bool IsEOF(const struct Token* const pToken)
{
	assert(pToken != NULL);
	return (pToken->kind == TK_EOF);
}
// トークンを作成
struct Token* CreateNewToken(void)
{
	struct Token* const pNewToken = (struct Token*)malloc(1 * sizeof(struct Token));
	assert(pNewToken != NULL);
	pNewToken->kind = TK_NONE;
	pNewToken->value = 0;
	pNewToken->next = NULL;
	pNewToken->str = NULL;
	return pNewToken;
}
void SetToken(struct Token* const pToken, const enum TokenKind kind, struct Token* const pNext, const int value, const char* const pStr)
{
	assert(pToken != NULL);
	pToken->kind = kind;
	pToken->next = pNext;
	pToken->value = value;
	pToken->str = pStr;
}
// 入力文字列をトークナイズ(トークンに分解)
struct Token* Tokenize(char* pStr)
{
	char* pStrFirst = pStr;

	struct Token head;
	head.next = NULL;
	struct Token* pCurrent = &head;

	while(pStr[0] != '\0')
	{
		// 空白文字をスキップ
		if (isspace(pStr[0]))
		{
			++pStr; // 次のトークンへ
			continue;
		}

		const int ch = (int)pStr[0];
		if (strchr("+-*/()", ch) != NULL)
		{
			struct Token* const pTmp = CreateNewToken();
			SetToken(&(*pTmp), TK_RESERVED, NULL, 0, pStr);
			pCurrent->next = pTmp;
			pCurrent = pTmp;
			++pStr; // 次のトークンへ
			continue;
		}

		if (isdigit(ch))
		{
			struct Token* const pTmp = CreateNewToken();
			SetToken(&(*pTmp), TK_NUM, NULL, strtol(pStr, &pStr, 10), pStr);
			pCurrent->next = pTmp;
			pCurrent = pTmp;
			continue;
		}

		ErrorAt(pStr, pStrFirst, "Cannot tokenize.");
	}

	struct Token* const pTail = CreateNewToken();
	SetToken(&(*pTail), TK_EOF, NULL, 0, pStr);
	pCurrent->next = pTail;
	return head.next;
}
void ReleaseTokenMemory(struct Token* pToken)
{
	struct Token* pDummy = pToken;
	while(pToken != NULL)
	{
		pDummy = pToken->next;
		free(pToken);
		pToken = pDummy;
	}
}

// -- NODE --
struct Node* Expr(struct Token** pToken, const char* const pSrc);
struct Node* Mul(struct Token** pToken, const char* const pSrc);
struct Node* Primary(struct Token** pToken, const char* const pSrc);

struct Node* CreateNewNode(void)
{
	struct Node* const pNode = (struct Node*)malloc(1 * sizeof(struct Node));
	assert(pNode != NULL);
	pNode->kind = ND_NONE;
	pNode->pLhs = NULL;
	pNode->pRhs = NULL;
	return pNode;
}
void SetNode(struct Node* const pNode, const enum NodeKind kind, struct Node* const pLhs, struct Node* const pRhs, const int value)
{
	assert(pNode != NULL);
	pNode->kind = kind;
	pNode->pLhs = pLhs;
	pNode->pRhs = pRhs;
	pNode->value = value;
}
// expr = mul ( "+" mul | "-" mul) *
struct Node* Expr(struct Token** pToken, const char* const pSrc)
{
	struct Node* pNode = Mul(&(*pToken), pSrc);
	while(true) // 0回以上の繰り返し
	{
		if (IsExpectedToken('+', *pToken))
		{
			*pToken = (*pToken)->next;

			struct Node* const pTmp = CreateNewNode();
			SetNode(&(*pTmp), ND_ADD, pNode, Mul(&(*pToken), pSrc), 0);
			pNode = pTmp;
		}
		else if (IsExpectedToken('-', *pToken))
		{
			*pToken = (*pToken)->next;

			struct Node* const pTmp = CreateNewNode();
			SetNode(&(*pTmp), ND_SUB, pNode, Mul(&(*pToken), pSrc), 0);
			pNode = pTmp;
		}
		else { break; }
	}
	return pNode;
}
// mul = primary ( "*" primary | "/" primary ) *
struct Node* Mul(struct Token** pToken, const char* const pSrc)
{
	struct Node* pNode = Primary(&(*pToken), pSrc);
	while(true) // 0回以上の繰り返し
	{
		if (IsExpectedToken('*', *pToken))
		{
			*pToken = (*pToken)->next;

			struct Node* const pTmp = CreateNewNode();
			SetNode(&(*pTmp), ND_MUL, pNode, Primary(&(*pToken), pSrc), 0);
			pNode = pTmp;
		}
		else if (IsExpectedToken('/', *pToken))
		{
			*pToken = (*pToken)->next;

			struct Node* const pTmp = CreateNewNode();
			SetNode(&(*pTmp), ND_DIV, pNode, Primary(&(*pToken), pSrc), 0);
			pNode = pTmp;
		}
		else { break; }
	}
	return pNode;
}
// primary = num | "(" expr ")"
struct Node* Primary(struct Token** pToken, const char* const pSrc)
{
	if (IsExpectedToken('(', *pToken))
	{
		*pToken = (*pToken)->next;

		struct Node* const pNode = Expr(&(*pToken), pSrc);
		if (!IsExpectedToken(')', *pToken)) { ErrorAt((*pToken)->str, pSrc, "need token ')'."); }
		*pToken = (*pToken)->next;
		return pNode;
	}

	if (!IsExpectedNumber(*pToken)) { ErrorAt((*pToken)->str, pSrc, "need token num"); }
	struct Node* const pNode = CreateNewNode();
	SetNode(&(*pNode), ND_NUM, NULL, NULL, (*pToken)->value);
	*pToken = (*pToken)->next;
	return pNode;
}
void ReleaseNodeMemory(struct Node* pRootNode)
{
	if (pRootNode == NULL) { return; }
	ReleaseNodeMemory(pRootNode->pLhs);
	ReleaseNodeMemory(pRootNode->pRhs);
	pRootNode->pLhs = NULL;
	pRootNode->pRhs = NULL;
	free(pRootNode);
}

int main(int argc, char *argv[])
{
	if (argc != 2)
	{
		fprintf(stderr, "This program requires more than two arguments.\n");
		return 1;
	}

	char* const userInput = argv[1];

	// トークナイズ
	struct Token* pToken = Tokenize(userInput);
	struct Token* pHead = pToken;
	// printf("\ntest token\n"); DebugPrintTokens(pHead);
	struct Node* pNode = Expr(&pToken, userInput);
	printf("\ntest node\n"); DebugPrintNodes(pNode);

	pToken = pHead;
	printf(".intel_syntax noprefix\n");
	printf(".global main\n");

	printf("main:\n");

	printf("        mov rax, %d\n", pToken->value);
	pToken = pToken->next;
	while (!IsEOF(pToken))
	{
		if (IsExpectedToken('+', pToken))
		{
			pToken = pToken->next;
			if (!IsExpectedNumber(pToken)) { ErrorAt(pToken->str, userInput, "Not number."); }
			printf("        add rax, %d\n", pToken->value);
		}
		else if (IsExpectedToken('-',pToken))
		{
			pToken = pToken->next;
			if (!IsExpectedNumber(pToken)) { ErrorAt(pToken->str, userInput, "Not number."); }
			printf("        sub rax, %d\n", pToken->value);
		}
		else
		{
			ReleaseTokenMemory(pHead);
			ReleaseNodeMemory(pNode);
			fprintf(stderr, "Cannot output assembly language.\n");
			return 1;
		}

		pToken = pToken->next;
	}

	printf("        ret\n");

	ReleaseTokenMemory(pHead);
	ReleaseNodeMemory(pNode);

	return 0;
}
