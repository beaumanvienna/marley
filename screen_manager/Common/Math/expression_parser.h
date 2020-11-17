#pragma once

#include <cstdint>
#include <vector>

typedef std::pair<uint32_t, uint32_t> ExpressionPair;
typedef std::vector<ExpressionPair> PostfixExpression;

enum ExpressionType
{
	EXPR_TYPE_UINT = 0,
	EXPR_TYPE_FLOAT = 2,
};

class SCREEN_IExpressionFunctions
{
public:
	virtual bool parseReference(char* str, uint32_t& referenceIndex) = 0;
	virtual bool parseSymbol(char* str, uint32_t& symbolValue) = 0;
	virtual uint32_t getReferenceValue(uint32_t referenceIndex) = 0;
	virtual ExpressionType getReferenceType(uint32_t referenceIndex) = 0;
	virtual bool getMemoryValue(uint32_t address, int size, uint32_t& dest, char* error) = 0;
};

bool initPostfixExpression(const char* infix, SCREEN_IExpressionFunctions* funcs, PostfixExpression& dest);
bool parsePostfixExpression(PostfixExpression& exp, SCREEN_IExpressionFunctions* funcs, uint32_t& dest);
bool parseExpression(const char* exp, SCREEN_IExpressionFunctions* funcs, uint32_t& dest);
const char* getExpressionError();
