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

// -- DEBUG --
// トークン構造体表示
void DebugPrintToken(const struct Token* const pToken)
{
	printf("Token Info: %p\n", pToken);
	printf("enum : %d\n", pToken->kind);
	printf("next : %p\n", pToken->next);
	printf("value: %d\n", pToken->value);
	printf("str  : %c(%d)\n", *(pToken->str), (int)(*pToken->str));
	printf("\n");
}
void DebugPrintTokens(const struct Token* pToken)
{
	while(pToken != NULL)
	{
		DebugPrintToken(pToken);
		pToken = pToken->next;
	}
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
struct Token* CreateNewToken(const enum TokenKind kind, const char* const str)
{
	struct Token* const pNewToken = (struct Token*)malloc(1 * sizeof(struct Token));
	assert(pNewToken != NULL);
	pNewToken->kind = kind;
	pNewToken->value = 0;
	pNewToken->next = NULL;
	pNewToken->str = str;
	return pNewToken;
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

		const char ch = pStr[0];
		if (ch == '+' || ch == '-')
		{
			struct Token* const pTmp = CreateNewToken(TK_RESERVED, pStr);
			pCurrent->next = pTmp;
			pCurrent = pTmp;
			++pStr; // 次のトークンへ
			continue;
		}

		if (isdigit(pStr[0]))
		{
			struct Token* const pTmp = CreateNewToken(TK_NUM, pStr);
			pCurrent->next = pTmp;
			pCurrent = pTmp;
			pCurrent->value = strtol(pStr, &pStr, 10); // 次のトークンへ
			continue;
		}

		ErrorAt(pStr, pStrFirst, "Cannot tokenize.");
	}

	struct Token* const pTail = CreateNewToken(TK_EOF, pStr);
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

int main(int argc, char *argv[])
{
	if (argc != 2)
	{
		fprintf(stderr, "This program requires more than two arguments.\n");
		return 1;
	}

	// トークナイズ
	struct Token* pToken = Tokenize(argv[1]);
	struct Token* pHead = pToken;

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
			if (!IsExpectedNumber(pToken)) { ErrorAt(pToken->str, argv[1], "Not number."); }
			printf("        add rax, %d\n", pToken->value);
		}
		else if (IsExpectedToken('-',pToken))
		{
			pToken = pToken->next;
			if (!IsExpectedNumber(pToken)) { ErrorAt(pToken->str, argv[1], "Not number."); }
			printf("        sub rax, %d\n", pToken->value);
		}
		else
		{
			fprintf(stderr, "Cannot output assembly language.\n");
			exit(1);
		}

		pToken = pToken->next;
	}

	printf("        ret\n");

	ReleaseTokenMemory(pHead);

	return 0;
}
