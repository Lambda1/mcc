#include <ctype.h>

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdarg.h>

#include <string.h>
#include <assert.h>

#include "mcc.h"

int tokenMemoryCount = 0;
int nodeMemoryCount = 0;
int lvarMemoryCount = 0;

// -- DEBUG --
// トークン構造体表示
void DebugPrintToken(const struct Token* const pToken)
{
	const char array[] = {'X', 'R', 'I', 'N', 'E', 'r', 'i', 'e', 'W', '{'};
	assert(pToken->kind < (sizeof(array)/sizeof(const char)));
	printf("Token Info: %p\n", pToken);
	printf("enum : %c\n", array[pToken->kind]);
	printf("next : %p\n", pToken->next);
	printf("value: %d\n", pToken->value);
	printf("str  : %s(%d)\n", pToken->str, (int)(*pToken->str));
	printf("len  : %d\n", pToken->len);
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
	const char array[] = {'X', '+', '-', '*', '/', 'n', 'a', 'v', '=', '!', '>', 'L', 'r', 'i' ,'w', '{', 'f'};
	assert(pNode->kind < (sizeof(array)/sizeof(const char)));
	printf("Node Info: %p\n", pNode);
	printf("kind  : %c(%d)\n", array[pNode->kind], pNode->kind);
	printf("pLhs  : %p\n", pNode->pLhs);
	printf("pRhs  : %p\n", pNode->pRhs);
	printf("value : %d\n", pNode->value);
	printf("offset: %d\n", pNode->offset);
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
bool IsAlphabetOrNumber(const char ch)
{
	return ('a' <= ch && ch <= 'z') || ('A' <= ch && ch <= 'Z') || ('0' <= ch && ch <= '9') || (ch == '_');
}

// -- TOKEN --
// トークンが期待する記号か判定
bool IsExpectedToken(const char* const op, const struct Token* const pToken)
{
	assert(pToken != NULL);
	if (pToken->kind != TK_RESERVED ||
		strlen(op) != pToken->len ||
		memcmp(pToken->str, op, pToken->len) != 0)
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
// トークンが期待する変数か判定
bool IsExpectedIdent(const struct Token* const pToken)
{
	assert(pToken != NULL);
	if (pToken->kind != TK_IDENT)
	{
		return false;
	}
	return true;
}
bool IsExpectedTokenForKey(const struct Token* const pToken, const enum TokenKind kind)
{
	assert(pToken != NULL);
	if (pToken->kind != kind) { return false; }
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
	++tokenMemoryCount;
	return pNewToken;
}
void SetToken(struct Token* const pToken, const enum TokenKind kind, struct Token* const pNext, const int value, const char* const pStr, const int len)
{
	assert(pToken != NULL);
	pToken->kind = kind;
	pToken->next = pNext;
	pToken->value = value;
	pToken->str = pStr;
	pToken->len = len;
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

		if (memcmp(pStr, "==", 2) == 0
			|| memcmp(pStr, "!=", 2) == 0
			|| memcmp(pStr, "<=", 2) == 0
			|| memcmp(pStr, ">=", 2) == 0)
		{
			struct Token* const pTmp = CreateNewToken();
			SetToken(&(*pTmp), TK_RESERVED, NULL, 0, pStr, 2);
			pCurrent->next = pTmp;
			pCurrent = pTmp;

			pStr += 2;
			continue;
		}

		const int ch = (int)pStr[0];
		if (strchr("+-*/()><=;{}", ch) != NULL)
		{
			struct Token* const pTmp = CreateNewToken();
			SetToken(&(*pTmp), TK_RESERVED, NULL, 0, pStr, 1);
			pCurrent->next = pTmp;
			pCurrent = pTmp;
			++pStr; // 次のトークンへ
			continue;
		}

		if (isdigit(ch))
		{
			struct Token* const pTmp = CreateNewToken();
			const char* pOriginStr = pStr;
			const int value = (int)strtol(pStr, &pStr, 10);
			SetToken(&(*pTmp), TK_NUM, NULL, value, pStr, (int)(pStr-pOriginStr));
			pCurrent->next = pTmp;
			pCurrent = pTmp;
			continue;
		}

		const int returnStrSize = strlen("return");
		bool isOver = (strlen(pStr) <= returnStrSize);
		if (!isOver && strncmp(pStr, "return", returnStrSize) == 0 && !IsAlphabetOrNumber(pStr[returnStrSize]))
		{
			struct Token* const pTmp = CreateNewToken();
			SetToken(&(*pTmp), TK_RETURN, NULL, 0, pStr, returnStrSize);
			pCurrent->next = pTmp;
			pCurrent = pTmp;
			pStr += returnStrSize;
			continue;
		}

		const int ifStrSize = strlen("if");
		isOver = (strlen(pStr) <= ifStrSize);
		if (!isOver && strncmp(pStr, "if", ifStrSize) == 0 && !IsAlphabetOrNumber(pStr[ifStrSize]))
		{
			struct Token* const pTmp = CreateNewToken();
			SetToken(&(*pTmp), TK_IF, NULL, 0, pStr, ifStrSize);
			pCurrent->next = pTmp;
			pCurrent = pTmp;
			pStr += ifStrSize;
			continue;
		}

		const int elseStrSize = strlen("else");
		isOver = (strlen(pStr) <= elseStrSize);
		if (!isOver && strncmp(pStr, "else", ifStrSize) == 0 && !IsAlphabetOrNumber(pStr[elseStrSize]))
		{
			struct Token* const pTmp = CreateNewToken();
			SetToken(&(*pTmp), TK_ELSE, NULL, 0, pStr, elseStrSize);
			pCurrent->next = pTmp;
			pCurrent = pTmp;
			pStr += elseStrSize;
			continue;
		}

		const int whileStrSize = strlen("while");
		isOver = (strlen(pStr) <= whileStrSize);
		if (!isOver && strncmp(pStr, "while", whileStrSize) == 0 && !IsAlphabetOrNumber(pStr[whileStrSize]))
		{
			struct Token* const pTmp = CreateNewToken();
			SetToken(&(*pTmp), TK_WHILE, NULL, 0, pStr, whileStrSize);
			pCurrent->next = pTmp;
			pCurrent = pTmp;
			pStr += whileStrSize;
			continue;
		}

		if (ch >= 'a' && ch <= 'z')
		{
			int i = 0;
			while(true)
			{
				if (pStr[i] < 'a' || pStr[i] > 'z') break;
				++i;
			}
			struct Token* const pTmp = CreateNewToken();
			SetToken(&(*pTmp), TK_IDENT, NULL, 0, pStr, i);
			pCurrent->next = pTmp;
			pCurrent = pTmp;
			pStr += i;
			continue;
		}

		ErrorAt(pStr, pStrFirst, "Cannot tokenize.");
	}

	struct Token* const pTail = CreateNewToken();
	SetToken(&(*pTail), TK_EOF, NULL, 0, pStr, 0);
	pCurrent->next = pTail;
	return head.next;
}
int ReleaseTokenMemory(struct Token* pToken)
{
	struct Token* pDummy = pToken;
	while(pToken != NULL)
	{
		pDummy = pToken->next;
		free(pToken);
		pToken = pDummy;
		--tokenMemoryCount;
	}
	return tokenMemoryCount;
}

// -- NODE --
struct Node* CreateNewNode(void)
{
	struct Node* const pNode = (struct Node*)malloc(1 * sizeof(struct Node));
	assert(pNode != NULL);
	pNode->kind = ND_NONE;
	pNode->pLhs = NULL;
	pNode->pRhs = NULL;
	pNode->pCond = NULL;
	pNode->pThen = NULL;
	pNode->pElse = NULL;
	pNode->pBlock = NULL;
	pNode->isReadBlock = 0;
	++nodeMemoryCount;
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
// program = stmt*
void Program(struct Node* codes[], struct Token** pToken, const char* const pSrc, struct LocalVar* const pFirstLVar)
{
	int i = 0;
	while(IsEOF(*pToken) != true)
	{
		codes[i] = Stmt(&(*pToken), pSrc, pFirstLVar);
		++i;
	}
	codes[i] = NULL;
}
// stmt = expr ";" | "{" stmt* "}" | "return" expr ";" | "if" "(" expr ")" stmt ("else" stmt)? | "while" "(" expr ")" stmt
struct Node* Stmt(struct Token** pToken, const char* const pSrc, struct LocalVar* const pFirstLVar)
{
	struct Node* pNode = NULL;
	if (IsExpectedTokenForKey(*pToken, TK_RETURN))
	{
		*pToken = (*pToken)->next;

		pNode = CreateNewNode();
		SetNode(&(*pNode), ND_RTN, Expr(&(*pToken), pSrc, pFirstLVar), NULL, 0);
	}
	else if (IsExpectedToken("{", *pToken))
	{
		*pToken = (*pToken)->next;
		pNode = CreateNewNode();
		SetNode(&(*pNode), ND_BLOCK, NULL, NULL, 0);
		struct Node* pHead = pNode;
		while(!IsExpectedToken("}", *pToken))
		{
			pNode->pBlock = Stmt(&(*pToken), pSrc, pFirstLVar);
			pNode = pNode->pBlock;
		}
		*pToken = (*pToken)->next;
		return pHead;
	}
	else if (IsExpectedTokenForKey(*pToken, TK_WHILE))
	{
		*pToken = (*pToken)->next;

		pNode = CreateNewNode();
		SetNode(&(*pNode), ND_WHILE, NULL, NULL, 0);

		if (!IsExpectedToken("(", *pToken)) { ErrorAt((*pToken)->str, pSrc, "need token '('."); }
		*pToken = (*pToken)->next;
		pNode->pCond = Expr(&(*pToken), pSrc, pFirstLVar);
		if (!IsExpectedToken(")", *pToken)) { ErrorAt((*pToken)->str, pSrc, "need token ')'."); }
		*pToken = (*pToken)->next;
		pNode->pThen = Stmt(&(*pToken), pSrc, pFirstLVar);
		return pNode;
	}
	else if (IsExpectedTokenForKey(*pToken, TK_IF))
	{
		*pToken = (*pToken)->next;

		pNode = CreateNewNode();
		SetNode(&(*pNode), ND_IF, NULL, NULL, 0);

		// "if" "(" expr ")"
		if (!IsExpectedToken("(", *pToken)) { ErrorAt((*pToken)->str, pSrc, "need token '('."); }
		*pToken = (*pToken)->next;
		pNode->pCond = Expr(&(*pToken), pSrc, pFirstLVar);
		if (!IsExpectedToken(")", *pToken)) { ErrorAt((*pToken)->str, pSrc, "need token ')'."); }
		*pToken = (*pToken)->next;
		// stmt
		pNode->pThen = Stmt(&(*pToken), pSrc, pFirstLVar);
		// ("else" stmt)?
		if (IsExpectedTokenForKey(*pToken, TK_ELSE))
		{
			*pToken = (*pToken)->next;
			pNode->pElse = Stmt(&(*pToken), pSrc, pFirstLVar);
		}
		return pNode;
	}
	else
	{
		pNode = Expr(&(*pToken), pSrc, pFirstLVar);
	}

	if (!IsExpectedToken(";", *pToken)) { ErrorAt((*pToken)->str, pSrc, "need token ';'."); }
	*pToken = (*pToken)->next;

	return pNode;
}
// expr = Assign
struct Node* Expr(struct Token** pToken, const char* const pSrc, struct LocalVar* const pFirstLVar)
{
	return Assign(&(*pToken), pSrc, pFirstLVar);
}
// assign = equality ("=" assign)?
struct Node* Assign(struct Token** pToken, const char* const pSrc, struct LocalVar* const pFirstLVar)
{
	struct Node* pNode = Equality(&(*pToken), pSrc, pFirstLVar);
	if (IsExpectedToken("=", *pToken))
	{
		*pToken = (*pToken)->next;

		struct Node* const pTmp = CreateNewNode();
		SetNode(&(*pTmp), ND_ASSIGN, pNode, Assign(&(*pToken), pSrc, pFirstLVar), 0);
		pNode = pTmp;
	}
	return pNode;
}
// equality = relational ("==" relational | "!=" relational)*
struct Node* Equality(struct Token** pToken, const char* const pSrc, struct LocalVar* const pFirstLVar)
{
	struct Node* pNode = Relational(&(*pToken), pSrc, pFirstLVar);
	while(true)
	{
		if (IsExpectedToken("==", *pToken))
		{
			*pToken = (*pToken)->next;

			struct Node* const pTmp = CreateNewNode();
			SetNode(&(*pTmp), ND_EQU, pNode, Relational(&(*pToken), pSrc, pFirstLVar), 0);
			pNode = pTmp;
		}
		else if (IsExpectedToken("!=", *pToken))
		{
			*pToken = (*pToken)->next;

			struct Node* const pTmp = CreateNewNode();
			SetNode(&(*pTmp), ND_NEQ, pNode, Relational(&(*pToken), pSrc, pFirstLVar), 0);
			pNode = pTmp;
		}
		else { break; }
	}
	return pNode;
}
// relational = add ("<" add | "<=" add | ">" add | ">=" add)*
struct Node* Relational(struct Token** pToken, const char* const pSrc, struct LocalVar* const pFirstLVar)
{
	struct Node* pNode = Add(&(*pToken), pSrc, pFirstLVar);
	while(true)
	{
		if (IsExpectedToken("<", *pToken))
		{
			*pToken = (*pToken)->next;

			struct Node* const pTmp = CreateNewNode();
			SetNode(&(*pTmp), ND_LTH, pNode, Add(&(*pToken), pSrc, pFirstLVar), 0);
			pNode = pTmp;
		}
		else if (IsExpectedToken("<=", *pToken))
		{
			*pToken = (*pToken)->next;

			struct Node* const pTmp = CreateNewNode();
			SetNode(&(*pTmp), ND_LEQ, pNode, Add(&(*pToken), pSrc, pFirstLVar), 0);
			pNode = pTmp;
		}
		else if (IsExpectedToken(">", *pToken))
		{
			*pToken = (*pToken)->next;

			struct Node* const pTmp = CreateNewNode();
			SetNode(&(*pTmp), ND_LTH, Relational(&(*pToken), pSrc, pFirstLVar), pNode, 0);
			pNode = pTmp;
		}
		else if (IsExpectedToken(">=", *pToken))
		{
			*pToken = (*pToken)->next;

			struct Node* const pTmp = CreateNewNode();
			SetNode(&(*pTmp), ND_LEQ, Relational(&(*pToken), pSrc, pFirstLVar), pNode, 0);
			pNode = pTmp;
		}
		else { break; }
	}
	return pNode;
}
// add = mul ("+" mul | "-" mul)*
struct Node* Add(struct Token** pToken, const char* const pSrc, struct LocalVar* const pFirstLVar)
{
	struct Node* pNode = Mul(&(*pToken), pSrc, pFirstLVar);
	while(true)
	{
		if (IsExpectedToken("+", *pToken))
		{
			*pToken = (*pToken)->next;

			struct Node* const pTmp = CreateNewNode();
			SetNode(&(*pTmp), ND_ADD, pNode, Mul(&(*pToken), pSrc, pFirstLVar), 0);
			pNode = pTmp;
		}
		else if (IsExpectedToken("-", *pToken))
		{
			*pToken = (*pToken)->next;

			struct Node* const pTmp = CreateNewNode();
			SetNode(&(*pTmp), ND_SUB, pNode, Mul(&(*pToken), pSrc, pFirstLVar), 0);
			pNode = pTmp;
		}
		else { break; }
	}
	return pNode;
}
// mul = unary ( "*" unary | "/" unary ) *
struct Node* Mul(struct Token** pToken, const char* const pSrc, struct LocalVar* const pFirstLVar)
{
	struct Node* pNode = Unary(&(*pToken), pSrc, pFirstLVar);
	while(true) // 0回以上の繰り返し
	{
		if (IsExpectedToken("*", *pToken))
		{
			*pToken = (*pToken)->next;

			struct Node* const pTmp = CreateNewNode();
			SetNode(&(*pTmp), ND_MUL, pNode, Unary(&(*pToken), pSrc, pFirstLVar), 0);
			pNode = pTmp;
		}
		else if (IsExpectedToken("/", *pToken))
		{
			*pToken = (*pToken)->next;

			struct Node* const pTmp = CreateNewNode();
			SetNode(&(*pTmp), ND_DIV, pNode, Unary(&(*pToken), pSrc, pFirstLVar), 0);
			pNode = pTmp;
		}
		else { break; }
	}
	return pNode;
}
// unary = ("+" | "-" )? primary
struct Node* Unary(struct Token** pToken, const char* const pSrc, struct LocalVar* const pFirstLVar)
{
	if (IsExpectedToken("+", *pToken))
	{
		*pToken = (*pToken)->next;

		struct Node* pNode = Primary(&(*pToken), pSrc, pFirstLVar);
		SetNode(&(*pNode), ND_NUM, NULL, NULL, 0);
		return pNode;
	}
	else if (IsExpectedToken("-", *pToken))
	{
		*pToken = (*pToken)->next;

		struct Node* const pLhs = CreateNewNode();
		SetNode(&(*pLhs), ND_NUM, NULL, NULL, 0);

		struct Node* const pNode = CreateNewNode();
		SetNode(&(*pNode), ND_SUB, pLhs, Primary(&(*pToken), pSrc, pFirstLVar), 0);
		return pNode;
	}
	return Primary(&(*pToken), pSrc, pFirstLVar);
}
// primary = num | ident | ident ("(" ident? ")")? | "(" expr ")"
struct Node* Primary(struct Token** pToken, const char* const pSrc, struct LocalVar* const pFirstLVar)
{
	if (IsExpectedToken("(", *pToken))
	{
		*pToken = (*pToken)->next;

		struct Node* const pNode = Expr(&(*pToken), pSrc, pFirstLVar);
		if (!IsExpectedToken(")", *pToken)) { ErrorAt((*pToken)->str, pSrc, "need token ')'."); }
		*pToken = (*pToken)->next;
		return pNode;
	}
	else if (IsExpectedIdent(*pToken))
	{
		struct Node* const pNode = CreateNewNode();

		// function
		if (IsExpectedToken("(", (*pToken)->next))
		{
			SetNode(&(*pNode), ND_FUNC, NULL, NULL, 0);
			pNode->pLabel = (*pToken)->str;
			pNode->labelLen = (*pToken)->len;
			*pToken = (*pToken)->next; // fuction-name
			*pToken = (*pToken)->next; // "("

			// argument
			if (IsExpectedIdent(*pToken))
			{
				exit(1); // そのうち作る
			}

			if (!IsExpectedToken(")", *pToken)) { ErrorAt((*pToken)->str, pSrc, "need token ')'."); }
			*pToken = (*pToken)->next;
			return pNode;
		}

		SetNode(&(*pNode), ND_LVAR, NULL, NULL, 0);
		const struct LocalVar* pLVar = FindLocalVar(pFirstLVar, *pToken);
		if (pLVar == NULL)
		{
			struct LocalVar* const pTmp = (struct LocalVar*)malloc(1 * sizeof(struct LocalVar));
			++lvarMemoryCount;
			struct LocalVar* pLastLVar = GetLastLocalVar(pFirstLVar);
			pTmp->next = NULL;
			pTmp->name = (*pToken)->str;
			pTmp->len = (*pToken)->len;
			pTmp->offset = pLastLVar->offset + 8/*Bytes*/;

			pLastLVar->next = pTmp;
			pNode->offset = pTmp->offset;
		}
		else
		{
			pNode->offset = pLVar->offset;
		}
		*pToken = (*pToken)->next;
		return pNode;
	}

	if (!IsExpectedNumber(*pToken)) { ErrorAt((*pToken)->str, pSrc, "need token num"); }
	struct Node* const pNode = CreateNewNode();
	SetNode(&(*pNode), ND_NUM, NULL, NULL, (*pToken)->value);
	*pToken = (*pToken)->next;
	return pNode;
}
int ReleaseNodeMemory(struct Node* pRootNode)
{
	if (pRootNode == NULL) { return nodeMemoryCount; }

	ReleaseNodeMemory(pRootNode->pBlock);
	ReleaseNodeMemory(pRootNode->pCond);
	ReleaseNodeMemory(pRootNode->pThen);
	ReleaseNodeMemory(pRootNode->pElse);

	ReleaseNodeMemory(pRootNode->pLhs);
	ReleaseNodeMemory(pRootNode->pRhs);

	free(pRootNode);
	--nodeMemoryCount;
	return nodeMemoryCount;
}

// -- LOCAL VARIABLE --
const struct LocalVar* FindLocalVar(const struct LocalVar* const pFirstLVar, const struct Token* const pToken)
{
	for (const struct LocalVar* pLVar = pFirstLVar; pLVar != NULL; pLVar = pLVar->next)
	{
		if ((pLVar->len == pToken->len) && !memcmp(pToken->str, pLVar->name, pLVar->len))
		{
			return pLVar;
		}
	}
	return NULL;
}
struct LocalVar* GetLastLocalVar(struct LocalVar* const pFirstLVar)
{
	struct LocalVar* pIndex = pFirstLVar;
	for (struct LocalVar* pLVar = pFirstLVar->next; pLVar != NULL; pLVar = pLVar->next)
	{
		pIndex = pLVar;
	}
	assert(pIndex != NULL);
	return pIndex;
}
int ReleaseLocalVarMemory(struct LocalVar* pLVar)
{
	struct LocalVar* pDummy = pLVar;
	while(pDummy != NULL)
	{
		pDummy = pLVar->next;
		free(pLVar);
		pLVar = pDummy;
		--lvarMemoryCount;
	}
	return lvarMemoryCount;
}
