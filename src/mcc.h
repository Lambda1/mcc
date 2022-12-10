// -- DEFINE --
// トークン: 単語
enum TokenKind
{
	TK_NONE,

	TK_RESERVED, // 記号
	TK_IDENT,    // 識別子
	TK_NUM,      // 整数トークン
	TK_EOF,      // 入力終了トークン

	// キーワード
	TK_RETURN,   // return文
	TK_IF,
	TK_ELSE,
	TK_WHILE,
};

struct Token
{
	enum TokenKind kind;
	struct Token *next;
	int value;
	const char* str; // トークン文字列
	int len;         // トークン長
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

	ND_ASSIGN,
	ND_LVAR,

	ND_EQU,
	ND_NEQ,

	ND_LTH,
	ND_LEQ,

	ND_RTN,

	ND_IF,
	ND_WHILE,

	ND_BLOCK,

	ND_FUNC,
};

struct Node
{
	enum NodeKind kind;
	struct Node* pLhs;
	struct Node* pRhs;
	int value;  // kind == ND_NUM
	int offset; // kind == ND_LVAR

	// if-else/while
	struct Node* pCond; // 条件
	struct Node* pThen; // 条件後の処理
	struct Node* pElse; // elseの処理

	struct Node* pBlock;
	int isReadBlock;

	const char* pLabel;
	int labelLen;
};

// ローカル変数
struct LocalVar
{
	struct LocalVar *next;
	const char* name; // 変数名
	int len;
	int offset;       // rbpからのオフセット
};

// -- Debug --
void DebugPrintNode(const struct Node* const pNode);
void DebugPrintTokens(const struct Token* pToken);
void DebugPrintNode(const struct Node* const pNode);
void DebugPrintNodes(const struct Node* const pRootNode);

// 本体
void ErrorAt(const char* const loc, const char* const userInput, char* fmt, ...);
bool IsAlphabetOrNumber(const char ch);

// -- Token --
bool IsExpectedToken(const char* const op, const struct Token* const pToken);
bool IsExpectedNumber(const struct Token* const pToken);
bool IsExpectedIdent(const struct Token* const pToken);
bool IsEOF(const struct Token* const pToken);
struct Token* CreateNewToken(void);
void SetToken(struct Token* const pToken, const enum TokenKind kind, struct Token* const pNext, const int value, const char* const pStr, const int len);
struct Token* Tokenize(char* pStr);
int ReleaseTokenMemory(struct Token* pToken);

// -- NODE --
struct Node* CreateNewNode(void);
void SetNode(struct Node* const pNode, const enum NodeKind kind, struct Node* const pLhs, struct Node* const pRhs, const int value);
void Program(struct Node* codes[], struct Token** pToken, const char* const pSrc, struct LocalVar* const pFirstLVar);
struct Node* Stmt(struct Token** pToken, const char* const pSrc, struct LocalVar* const pFirstLVar);
struct Node* Expr(struct Token** pToken, const char* const pSrc, struct LocalVar* const pFirstLVar);
struct Node* Assign(struct Token** pToken, const char* const pSrc, struct LocalVar* const pFirstLVar);
struct Node* Equality(struct Token** pToken, const char* const pSrc, struct LocalVar* const pFirstLVar);
struct Node* Relational(struct Token** pToken, const char* const pSrc, struct LocalVar* const pFirstLVar);
struct Node* Add(struct Token** pToken, const char* const pSrc, struct LocalVar* const pFirstLVar);
struct Node* Mul(struct Token** pToken, const char* const pSrc, struct LocalVar* const pFirstLVar);
struct Node* Unary(struct Token** pToken, const char* const pSrc, struct LocalVar* const pFirstLVar);
struct Node* Primary(struct Token** pToken, const char* const pSrc, struct LocalVar* const pFirstLVar);
int ReleaseNodeMemory(struct Node* pRootNode);

// -- LOCAL VARIABLE --
const struct LocalVar* FindLocalVar(const struct LocalVar* const pFirstLVar, const struct Token* const pToken);
struct LocalVar* GetLastLocalVar(struct LocalVar* const pFirstLVar);
int ReleaseLocalVarMemory(struct LocalVar* pLVar);

// -- CODE GENERATOR --
void GenLval(const struct Node* const pNode);
void Gen(const struct Node* const pNode);
