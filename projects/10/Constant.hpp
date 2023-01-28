#pragma once
#include <vector>

using namespace std;
enum class TokenType {
    KEYWORD,
    SYMBOL,
    IDENTIFIER,
    INT_CONST,
    STRING_CONST,
};

const vector<string> UNARY_OPERATORS = {"-", "~"};
const vector<string> BINARY_OPERATORS = {"+", "-", "*", "/", "&amp;", "|", "&lt;", "&gt;", "="};
const string COMMENT_LINE = "//";
const string COMMENT_START = "/*";
const string COMMENT_END = "*/";
const string CRSPACE = "\n\r\t ";
