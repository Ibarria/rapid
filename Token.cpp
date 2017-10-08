#include "Token.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

Token::~Token()
{
	if (type == STRING || type == IDENTIFIER) {
		if (pl.pstr) {
			free(pl.pstr);
			pl.pstr = nullptr;
		}
		type = INVALID;
	}
}

void Token::clear()
{
	if (type == STRING || type == IDENTIFIER) {
		if (pl.pstr) {
			free(pl.pstr);
			pl.pstr = nullptr;
		}
	}
	type = INVALID;
}

Token::Token(const Token & rhs)
{
	*this = rhs;
}

Token::Token(Token && rhs)
{
	*this = rhs;
}

Token & Token::operator=(Token && rhs)
{
	if (type == STRING || type == IDENTIFIER) {
		if (pl.pstr) {
			free(pl.pstr);
			pl.pstr = nullptr;
		}
	}
	type = rhs.type;
	pl = rhs.pl;
	rhs.pl.pstr = nullptr;
	return *this;
}

Token & Token::operator=(Token const & rhs)
{
	if (type == STRING || type == IDENTIFIER) {
		if (pl.pstr) {
			free(pl.pstr);
			pl.pstr = nullptr;
		}
	}
	type = rhs.type;
	if (type == STRING || type == IDENTIFIER) {
		u32 s = (u32)strlen(rhs.pl.pstr);
		pl.pstr = new char[s + 1];
		for (u32 i = 0; i < s; i++) pl.pstr[i] = rhs.pl.pstr[i];
	} else {
		pl = rhs.pl;
	}
	return *this;
}

void Token::print()
{
	printf("Token type %s", TokenTypeToStr(type));
	switch (type) {
	case NUMBER:
		printf(" %d", pl.pu32);
		break;
	case IDENTIFIER:
	case STRING:
		printf(" %s", pl.pstr);
		break;
	case CHAR:
		printf(" %c", pl.pu32);
		break;
	}
	printf("\n");
}

#define CASE_TOKEN_TYPE(a) case a: return #a
const char * TokenTypeToStr(TOKEN_TYPE type)
{
	switch (type) {
		CASE_TOKEN_TYPE(EOF);
		CASE_TOKEN_TYPE(NUMBER);
		CASE_TOKEN_TYPE(IDENTIFIER);
		CASE_TOKEN_TYPE(EQ);
		CASE_TOKEN_TYPE(LEQ);
		CASE_TOKEN_TYPE(GEQ);
		CASE_TOKEN_TYPE(NEQ);
		CASE_TOKEN_TYPE(LT);
		CASE_TOKEN_TYPE(GT);
		CASE_TOKEN_TYPE(RSHIFT);
		CASE_TOKEN_TYPE(LSHIFT);
		CASE_TOKEN_TYPE(ASSIGN);
		CASE_TOKEN_TYPE(OPEN_PAREN);
		CASE_TOKEN_TYPE(CLOSE_PAREN);
		CASE_TOKEN_TYPE(OPEN_BRACKET);
		CASE_TOKEN_TYPE(CLOSE_BRACKET);
		CASE_TOKEN_TYPE(OPEN_SQBRACKET);
		CASE_TOKEN_TYPE(CLOSE_SQBRACKET);
		CASE_TOKEN_TYPE(SEMICOLON);
		CASE_TOKEN_TYPE(COLON);
		CASE_TOKEN_TYPE(PERIOD);
		CASE_TOKEN_TYPE(HASH);
		CASE_TOKEN_TYPE(STAR);
		CASE_TOKEN_TYPE(DIV);
		CASE_TOKEN_TYPE(MOD);
		CASE_TOKEN_TYPE(AMP);
		CASE_TOKEN_TYPE(PLUS);
		CASE_TOKEN_TYPE(MINUS);
		CASE_TOKEN_TYPE(COMMA);
		CASE_TOKEN_TYPE(BANG);
		CASE_TOKEN_TYPE(STRING);
		CASE_TOKEN_TYPE(CHAR);
	}
	return "UNKNOWN";
}