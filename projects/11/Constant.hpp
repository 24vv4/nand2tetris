#pragma once
#include <map>
#include <vector>

using namespace std;
enum class TokenType {
    KEYWORD,
    SYMBOL,
    IDENTIFIER,
    INT_CONST,
    STRING_CONST,
};

enum class IDENTIFIER_KIND {
    STATIC,
    FIELD,
    ARG,
    VAR,
    NONE,
};

string printIdentifier(IDENTIFIER_KIND &kind) {
    switch(kind) {
        case IDENTIFIER_KIND::STATIC: return "static";
        case IDENTIFIER_KIND::FIELD:  return "field";
        case IDENTIFIER_KIND::ARG:    return "arg";
        case IDENTIFIER_KIND::VAR:    return "var";
        default: return "";
    }
};

enum class MEMORY_SEGMENT {
    CONST,
    ARG,
    LOCAL,
    STATIC,
    THIS,
    THAT,
    POINTER,
    TEMP,
};

enum class COMMAND {
    ADD,
    SUB,
    NEG,
    EQ,
    GT,
    LT,
    AND,
    OR,
    NOT,
    MUL,
    DIV,
};

map<string, COMMAND> commandMap = {
    {"+",     COMMAND::ADD},
    {"-",     COMMAND::SUB},
    {"u-",    COMMAND::NEG},
    {"=",     COMMAND::EQ},
    {"&gt;",  COMMAND::GT},
    {"&lt;",  COMMAND::LT},
    {"&amp;", COMMAND::AND},
    {"|",     COMMAND::OR},
    {"~",     COMMAND::NOT},
    {"*",     COMMAND::MUL},
    {"/",     COMMAND::DIV},
};

map<IDENTIFIER_KIND, MEMORY_SEGMENT> segmentMap = {
    {IDENTIFIER_KIND::FIELD, MEMORY_SEGMENT::THIS},
    {IDENTIFIER_KIND::STATIC, MEMORY_SEGMENT::STATIC},
    {IDENTIFIER_KIND::ARG, MEMORY_SEGMENT::ARG},
    {IDENTIFIER_KIND::VAR, MEMORY_SEGMENT::LOCAL},
};

const vector<string> UNARY_OPERATORS = {"-", "~"};
const vector<string> BINARY_OPERATORS = {"+", "-", "*", "/", "&amp;", "|", "&lt;", "&gt;", "="};
const string COMMENT_LINE = "//";
const string COMMENT_START = "/*";
const string COMMENT_END = "*/";
const string CRSPACE = "\n\r\t ";
