#pragma once

enum class CommandType {
    C_ARITHMETIC,
    C_PUSH,
    C_POP,
    C_LABEL,
    C_GOTO,
    C_IF,
    C_FUNCTION,
    C_RETURN,
    C_CALL
};

const int POINTER_BASE = 3;
const int TEMP_BASE = 5;

const std::string COMMENT = "//";
const std::string CRSPACE = "\n\r ";
