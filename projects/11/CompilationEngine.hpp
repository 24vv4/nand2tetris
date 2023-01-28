#include <filesystem>
#include <fstream>
#include <iostream>

#include "./Constant.hpp"
#include "./JackTokenizer.hpp"
#include "./SymbolTable.hpp"
#include "./VMWriter.hpp"

using namespace std;

class CompilationEngine {
public:
    CompilationEngine(string file_name, JackTokenizer& jack_tokenizer) 
        : jack_tokenizer_(jack_tokenizer), vmwriter_(VMWriter(file_name)) {
        int end = int(file_name.find(".jack"));
        file_name.replace(end, 5, ".xml");
        cout << file_name << endl;
        ofstream ofs(file_name);
        if(!ofs) {
            cout << "open write .xml file failed." << endl;
            return;
        }
        ofs_ = move(ofs);
    }

    void compileClass() {
        ofs_ << "<class>" << endl;
        assertWriteKeyword({"class"});
        // className
        assertWriteToken(TokenType::IDENTIFIER);
        vmwriter_.setClassName(jack_tokenizer_.identifier());
        assertWriteSymbol({"{"});
        // classVarDec*
        while(true) {
            if(lookaheadKeyword({"static", "field"})) {
                ofs_ << "<classVarDec>" << endl;
                assertWriteKeyword({"static", "field"});
                if(jack_tokenizer_.keyword() == "static") compileDec(IDENTIFIER_KIND::STATIC);
                else compileDec(IDENTIFIER_KIND::FIELD);
                ofs_ << "</classVarDec>" << endl;
            } else {
                break;
            }
        }
        // subroutineDec*
        while(true) {
            if(lookaheadKeyword({"constructor", "function", "method"})) {
                ofs_ << "<subroutineDec>" << endl;
                compileSubroutineDec();
                ofs_ << "</subroutineDec>" << endl;
            } else {
                break;
            }
        }
        assertWriteSymbol({"}"});
        ofs_ << "</class>" << endl;
    }

private:
    ofstream ofs_;
    JackTokenizer& jack_tokenizer_;
    SymbolTable symbol_table_;
    VMWriter vmwriter_;

    // type varName (',' varName)* ';'
    void compileDec(IDENTIFIER_KIND kind) {
        string type;
        compileType(type);
        assertWriteToken(TokenType::IDENTIFIER, true, kind);
        symbol_table_.define(jack_tokenizer_.identifier(), type, kind);
        while(true) {
            if(lookaheadSymbol({","})) {
                assertWriteSymbol({","});
                assertWriteToken(TokenType::IDENTIFIER, true, kind);
                symbol_table_.define(jack_tokenizer_.identifier(), type, kind);
            } else {
                break;
            }
        }
        assertWriteSymbol({";"});
    }

    void compileSubroutineDec() {
        assertWriteKeyword({"constructor", "function", "method"});
        string func_type = jack_tokenizer_.keyword();
        symbol_table_.startSubroutine(func_type);
        string _;
        // 'void' | type
        if(lookaheadKeyword({"void"})) {
            assertWriteKeyword({"void"});
        } else {
            compileType(_);
        }
        assertWriteToken(TokenType::IDENTIFIER);
        string subroutine_name = jack_tokenizer_.identifier();
        assertWriteSymbol({"("});
        compileParameterList();
        assertWriteSymbol({")"});
        ofs_ << "<subroutineBody>" << endl;
        assertWriteSymbol({"{"});
        while(true) {
            if(lookaheadKeyword({"var"})) {
                ofs_ << "<varDec>" << endl;
                assertWriteKeyword({"var"});
                compileDec(IDENTIFIER_KIND::VAR);
                ofs_ << "</varDec>" << endl;
            } else {
                break;
            }
        }
        vmwriter_.writeFunction(subroutine_name, symbol_table_.varCount(IDENTIFIER_KIND::VAR));
        if(func_type == "constructor") {
            // memory alloc for new instance
            vmwriter_.writePush(MEMORY_SEGMENT::CONST, symbol_table_.varCount(IDENTIFIER_KIND::FIELD));
            vmwriter_.writeMemoryAlloc();
            vmwriter_.writePop(MEMORY_SEGMENT::POINTER, 0);
        } else if(func_type == "method") {
            // set instance to RAM[THIS]
            vmwriter_.writePush(MEMORY_SEGMENT::ARG, 0);
            vmwriter_.writePop(MEMORY_SEGMENT::POINTER, 0);
        } else if(func_type == "function") {
            // nothing
        }
        compileStatements();
        assertWriteSymbol({"}"});
        ofs_ << "</subroutineBody>" << endl;
    }

    void compileType(string &type) {
        if(lookaheadKeyword({"int", "char", "boolean"})) {
            assertWriteKeyword({"int", "char", "boolean"});
            type = jack_tokenizer_.keyword();
        }
        else if(lookaheadToken(TokenType::IDENTIFIER)) {
            // className
            assertWriteToken(TokenType::IDENTIFIER);
            type = jack_tokenizer_.identifier();
        }
    }

    // ((type varName) (',' type varName)*)?
    void compileParameterList() {
        ofs_ << "<parameterList>" << endl;
        string type = "";
        if(!lookaheadType()) {
            ofs_ << "</parameterList>" << endl;
            return;
        }
        assertWriteToken(TokenType::IDENTIFIER, true, IDENTIFIER_KIND::ARG);
        symbol_table_.define(jack_tokenizer_.identifier(), type, IDENTIFIER_KIND::ARG);
        // (',' type varName)*
        while(true) {
            if(lookaheadSymbol({","})) {
                assertWriteSymbol({","});
                compileType(type);
                assertWriteToken(TokenType::IDENTIFIER, true, IDENTIFIER_KIND::ARG);
                symbol_table_.define(jack_tokenizer_.identifier(), type, IDENTIFIER_KIND::ARG);
            } else {
                break;
            }
        }
        ofs_ << "</parameterList>" << endl;
    }

    void compileStatements() {
        ofs_ << "<statements>" << endl;
        while(true) {
            if(!lookaheadKeyword({"let", "if", "while", "do", "return"})) break;
            if(lookaheadKeyword({"let"})) compileLet();
            if(lookaheadKeyword({"if"})) compileIf();
            if(lookaheadKeyword({"while"})) compileWhile();
            if(lookaheadKeyword({"do"})) compileDo();
            if(lookaheadKeyword({"return"})) compileReturn();
        }
        ofs_ << "</statements>" << endl;
    }

    void compileLet() {
        ofs_ << "<letStatement>" << endl;
        assertWriteKeyword({"let"});
        assertWriteToken(TokenType::IDENTIFIER, false, IDENTIFIER_KIND::VAR);
        // left value
        string left_token = jack_tokenizer_.identifier();
        IDENTIFIER_KIND kind = symbol_table_.kindOf(left_token);
        int index = symbol_table_.indexOf(left_token);
        bool array = false;
        // ('[' expression ']')?
        if(lookaheadSymbol({"["})) {
            assertWriteSymbol({"["});
            compileExpression();
            assertWriteSymbol({"]"});
            // array base address + i(=expression)
            array = true;
            vmwriter_.writePush(segmentMap[kind], index);
            vmwriter_.writeArithmetic(COMMAND::ADD);
        }
        assertWriteSymbol({"="});
        compileExpression();
        if(array) {
            // a[i] = y
            // y -> temp0, a[i] -> pointer1
            vmwriter_.writePop(MEMORY_SEGMENT::TEMP, 0);
            vmwriter_.writePop(MEMORY_SEGMENT::POINTER, 1);
            vmwriter_.writePush(MEMORY_SEGMENT::TEMP, 0);
            vmwriter_.writePop(MEMORY_SEGMENT::THAT, 0);
        } else {
            // a = y
            vmwriter_.writePop(segmentMap[kind], index);
        }
        assertWriteSymbol({";"});
        ofs_ << "</letStatement>" << endl;
    }

    void compileIf() {
        ofs_ << "<ifStatement>" << endl;
        assertWriteKeyword({"if"});
        assertWriteSymbol({"("});
        compileExpression();
        vmwriter_.writeArithmetic(commandMap["~"]);
        int label = vmwriter_.getLabel();
        vmwriter_.writeIf("IF_FALSE" + to_string(label));
        assertWriteSymbol({")"});
        assertWriteSymbol({"{"});
        compileStatements();
        assertWriteSymbol({"}"});
        vmwriter_.writeGoto("IF_END" + to_string(label));
        vmwriter_.writeLabel("IF_FALSE" + to_string(label));
        // ('else' '{' statements '}')?
        if(lookaheadKeyword({"else"})) {
            assertWriteKeyword({"else"});
            assertWriteSymbol({"{"});
            compileStatements();
            assertWriteSymbol({"}"});
        }
        vmwriter_.writeLabel("IF_END" + to_string(label));
        ofs_ << "</ifStatement>" << endl;
    }

    void compileWhile() {
        ofs_ << "<whileStatement>" << endl;
        assertWriteKeyword({"while"});
        assertWriteSymbol({"("});
        int label = vmwriter_.getLabel();
        vmwriter_.writeLabel("WHILE_EXP" + to_string(label));
        compileExpression();
        vmwriter_.writeArithmetic(commandMap["~"]);
        vmwriter_.writeIf("WHILE_END" + to_string(label));
        assertWriteSymbol({")"});
        assertWriteSymbol({"{"});
        compileStatements();
        assertWriteSymbol({"}"});
        vmwriter_.writeGoto("WHILE_EXP" + to_string(label));
        vmwriter_.writeLabel("WHILE_END" + to_string(label));
        ofs_ << "</whileStatement>" << endl;
    }

    void compileDo() {
        ofs_ << "<doStatement>" << endl;
        assertWriteKeyword({"do"});
        compileSubroutineCall();
        assertWriteSymbol({";"});
        ofs_ << "</doStatement>" << endl;
        // dispose return value
        vmwriter_.writePop(MEMORY_SEGMENT::TEMP, 0);
    }

    void compileReturn() {
        ofs_ << "<returnStatement>" << endl;
        assertWriteKeyword({"return"});
        if(lookaheadExpression()) compileExpression();
        else vmwriter_.writePush(MEMORY_SEGMENT::CONST, 0); // void function return 0
        assertWriteSymbol({";"});
        ofs_ << "</returnStatement>" << endl;
        vmwriter_.writeReturn();
    }

    void compileExpression() {
        ofs_ << "<expression>" << endl;
        compileTerm();
        while(true) {
            if(lookaheadSymbol(BINARY_OPERATORS)) {
                assertWriteSymbol(BINARY_OPERATORS);
                string op = jack_tokenizer_.symbol();
                compileTerm();
                vmwriter_.writeArithmetic(commandMap[op]);
            } else {
                break;
            }
        }
        ofs_ << "</expression>" << endl;
    }

    void compileTerm() {
        ofs_ << "<term>" << endl;
        if(lookaheadToken(TokenType::INT_CONST)) {
            assertWriteToken(TokenType::INT_CONST);
            vmwriter_.writePush(MEMORY_SEGMENT::CONST, jack_tokenizer_.intVal());
        } else if(lookaheadToken(TokenType::STRING_CONST)) {
            assertWriteToken(TokenType::STRING_CONST);
            string word = jack_tokenizer_.stringVal();
            vmwriter_.writePush(MEMORY_SEGMENT::CONST, word.size());
            vmwriter_.writeCall("String.new", 1);
            for(int i = 0; i < word.size(); i++) {
                vmwriter_.writePush(MEMORY_SEGMENT::CONST, int(word[i]));
                vmwriter_.writeCall("String.appendChar", 2);
            }
        } else if(lookaheadKeyword({"true", "false", "null", "this"})) {
            assertWriteKeyword({"true", "false", "null", "this"});
            string keyword = jack_tokenizer_.keyword();
            if(keyword == "true") {
                vmwriter_.writePush(MEMORY_SEGMENT::CONST, 1);
                vmwriter_.writeArithmetic(COMMAND::NEG);
            } else if(keyword == "false" || keyword == "null") {
                vmwriter_.writePush(MEMORY_SEGMENT::CONST, 0);
            } else if(keyword == "this") {
                vmwriter_.writePush(MEMORY_SEGMENT::POINTER, 0);
            }
        } else if(lookaheadToken(TokenType::IDENTIFIER)) {
            jack_tokenizer_.advance();
            if(lookaheadSymbol({"["})) {
                // varName '[' expression ']'
                jack_tokenizer_.retreat();
                assertWriteToken(TokenType::IDENTIFIER, false, IDENTIFIER_KIND::VAR);
                string token = jack_tokenizer_.identifier();
                IDENTIFIER_KIND kind = symbol_table_.kindOf(token);
                int index = symbol_table_.indexOf(token);
                assertWriteSymbol({"["});
                compileExpression();
                assertWriteSymbol({"]"});
                vmwriter_.writePush(segmentMap[kind], index);
                vmwriter_.writeArithmetic(COMMAND::ADD);
                // set a[i] address to pointer1, and push using THAT
                vmwriter_.writePop(MEMORY_SEGMENT::POINTER, 1);
                vmwriter_.writePush(MEMORY_SEGMENT::THAT, 0);
            } else if(lookaheadSymbol({"(", "."})) {
                // subroutineCall
                jack_tokenizer_.retreat();
                compileSubroutineCall();
            } else {
                // varName
                jack_tokenizer_.retreat();
                assertWriteToken(TokenType::IDENTIFIER, false, IDENTIFIER_KIND::VAR);
                string token = jack_tokenizer_.identifier();
                IDENTIFIER_KIND kind = symbol_table_.kindOf(token);
                int index = symbol_table_.indexOf(token);
                vmwriter_.writePush(segmentMap[kind], index);
            }
        } else if(lookaheadSymbol({"("})) {
            // '(' expression ')'
            assertWriteSymbol({"("});
            compileExpression();
            assertWriteSymbol({")"});
        } else if(lookaheadSymbol(UNARY_OPERATORS)) {
            // unaryOp term
            assertWriteSymbol(UNARY_OPERATORS);
            string op = jack_tokenizer_.symbol();
            // avoid collision with COMMAND::SUB
            if(op == "-") op = "u" + op;
            compileTerm();
            vmwriter_.writeArithmetic(commandMap[op]);
        }
        ofs_ << "</term>" << endl;
    }

    void compileSubroutineCall() {
        // subroutineName | className | varName
        assertWriteToken(TokenType::IDENTIFIER);
        string name = jack_tokenizer_.identifier();
        string subroutine = name;
        int n_args = 0;
        bool class_method = false;
        if(lookaheadSymbol({"."})) {
            IDENTIFIER_KIND kind = symbol_table_.kindOf(name);
            int index = symbol_table_.indexOf(name);
            string type = symbol_table_.typeOf(name);
            assertWriteSymbol({"."});
            // subroutineName
            assertWriteToken(TokenType::IDENTIFIER);
            if(kind == IDENTIFIER_KIND::NONE) {
                // when it is varName, it should be exist in symbolTable
                // therefore we can predict "name" is className
                subroutine = name + "." + jack_tokenizer_.identifier();
            } else {
                // varName
                vmwriter_.writePush(segmentMap[kind], index);
                n_args++;
                subroutine = type + "." + jack_tokenizer_.identifier();
            }
 
        } else {
            class_method = true;
            // when we call method we push instance at arg 0
            vmwriter_.writePush(MEMORY_SEGMENT::POINTER, 0);
            n_args++;
        }
        assertWriteSymbol({"("});
        n_args += compileExpressionList();
        assertWriteSymbol({")"});
        vmwriter_.writeCall(subroutine, n_args, class_method);
    }
       
    int compileExpressionList() {
        int n_args = 0;
        ofs_ << "<expressionList>" << endl;
        if(lookaheadExpression()) {
            compileExpression();
            n_args++;
            while(true) {
                if(lookaheadSymbol({","})) {
                    assertWriteSymbol({","});
                    compileExpression();
                    n_args++;
                } else {
                    break;
                }
            }
        }
        ofs_ << "</expressionList>" << endl;
        return n_args;
    }

    void assertWriteKeyword(vector<string> expected_keywords) {
        bool ok = false;
        jack_tokenizer_.advance();
        if(jack_tokenizer_.tokenType() == TokenType::KEYWORD) {
            for(string keyword : expected_keywords) {
                if(jack_tokenizer_.keyword() == keyword) ok = true;
            }
            if(ok) jack_tokenizer_.writeKeyword(ofs_);
        }
        if(!ok) compileError();
    }

    void assertWriteSymbol(vector<string> expected_symbols) {
        bool ok = false;
        jack_tokenizer_.advance();
        if(jack_tokenizer_.tokenType() == TokenType::SYMBOL) {
            for(string symbol : expected_symbols) {
                if(jack_tokenizer_.symbol() == symbol) ok = true;
            }
            if(ok) jack_tokenizer_.writeSymbol(ofs_);
        }
        if(!ok) compileError();
    }

    void assertWriteToken(TokenType expected_token_type, bool defined = true, IDENTIFIER_KIND kind = IDENTIFIER_KIND::STATIC) {
        jack_tokenizer_.advance();
        if(jack_tokenizer_.tokenType() != expected_token_type) {
            compileError();
        } else {
            jack_tokenizer_.writeToken(ofs_, expected_token_type, defined, kind);
        }
    }

    bool lookaheadKeyword(vector<string> expected_keywords) {
        bool match = false;
        jack_tokenizer_.advance();
        if(jack_tokenizer_.tokenType() == TokenType::KEYWORD) {
            for(string keyword : expected_keywords) {
                if(jack_tokenizer_.keyword() == keyword) match = true;
            }
        }
        jack_tokenizer_.retreat();
        return match;
    }

    bool lookaheadSymbol(vector<string> expected_symbols) {
        bool match = false;
        jack_tokenizer_.advance();
        if(jack_tokenizer_.tokenType() == TokenType::SYMBOL) {
            for(string symbol : expected_symbols) {
                if(jack_tokenizer_.symbol() == symbol) match = true;
            }
        }
        jack_tokenizer_.retreat();
        return match;
    }

    bool lookaheadToken(TokenType expected_token_type) {
        bool match = false;
        jack_tokenizer_.advance();
        if(jack_tokenizer_.tokenType() == expected_token_type) {
            match = true;
        }
        jack_tokenizer_.retreat();
        return match;
    }

    bool lookaheadExpression() {
        // term
        if(lookaheadToken(TokenType::INT_CONST)) return true;
        if(lookaheadToken(TokenType::STRING_CONST)) return true;
        if(lookaheadKeyword({"true", "false", "null", "this"})) return true;
        if(lookaheadToken(TokenType::IDENTIFIER)) return true;
        if(lookaheadSymbol({"("})) return true;
        if(lookaheadSymbol(UNARY_OPERATORS)) return true;
        return false;
    }

    bool lookaheadType() {
        if(lookaheadKeyword({"int", "char", "boolean"})) return true;
        if(lookaheadToken(TokenType::IDENTIFIER)) return true;
        return false;
    }

    void compileError() {
        // TODO: be more valuable message
        cout << "compilation failed" << endl;
    }
};
