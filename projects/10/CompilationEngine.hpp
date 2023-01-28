#include <filesystem>
#include <fstream>
#include <iostream>

#include "./JackTokenizer.hpp"
#include "./Constant.hpp"

using namespace std;

class CompilationEngine {
public:
    CompilationEngine(string file_name, JackTokenizer& jack_tokenizer) 
        : jack_tokenizer_(jack_tokenizer) {
        int end = int(file_name.find(".jack"));
        file_name.replace(end, 5, ".xml");
        cout << file_name << endl;
        ofstream ofs(file_name);
        if(!ofs) {
            cout << "open write file failed." << endl;
            return;
        }
        ofs_ = move(ofs);
    }

    void compileClass() {
        ofs_ << "<class>" << endl;
        assertWriteKeyword({"class"});
        assertWriteToken(TokenType::IDENTIFIER);
        assertWriteSymbol({"{"});
        while(true) {
            if(lookaheadKeyword({"static", "field"})) {
                ofs_ << "<classVarDec>" << endl;
                assertWriteKeyword({"static", "field"});
                compileDec();
                ofs_ << "</classVarDec>" << endl;
            } else {
                break;
            }
        }
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

    void compileDec() {
        compileType();
        assertWriteToken(TokenType::IDENTIFIER);
        while(true) {
            if(lookaheadSymbol({","})) {
                assertWriteSymbol({","});
                assertWriteToken(TokenType::IDENTIFIER);
            } else {
                break;
            }
        }
        assertWriteSymbol({";"});
    }

    void compileSubroutineDec() {
        assertWriteKeyword({"constructor", "function", "method"});
        // 'void' | type
        if(lookaheadKeyword({"void"})) {
            assertWriteKeyword({"void"});
        } else {
            compileType();
        }
        assertWriteToken(TokenType::IDENTIFIER);
        assertWriteSymbol({"("});
        compileParameterList();
        assertWriteSymbol({")"});
        ofs_ << "<subroutineBody>" << endl;
        assertWriteSymbol({"{"});
        while(true) {
            if(lookaheadKeyword({"var"})) {
                ofs_ << "<varDec>" << endl;
                assertWriteKeyword({"var"});
                compileDec();
                ofs_ << "</varDec>" << endl;
            } else {
                break;
            }
        }
        compileStatements();
        assertWriteSymbol({"}"});
        ofs_ << "</subroutineBody>" << endl;
    }

    bool compileType() {
        if(lookaheadKeyword({"int", "char", "boolean"})) {
            assertWriteKeyword({"int", "char", "boolean"});
            return true;
        }
        if(lookaheadToken(TokenType::IDENTIFIER)) {
            assertWriteToken(TokenType::IDENTIFIER);
            return true;
        }
        return false;
    }

    // ((type varName) (',' type varName)*)?
    void compileParameterList() {
        ofs_ << "<parameterList>" << endl;
        if(!compileType()) {
            ofs_ << "</parameterList>" << endl;
            return;
        }
        assertWriteToken(TokenType::IDENTIFIER);
        // (',' type varName)*
        while(true) {
            if(lookaheadSymbol({","})) {
                assertWriteSymbol({","});
                compileType();
                assertWriteToken(TokenType::IDENTIFIER);
            } else {
                break;
            }
        }
        ofs_ << "</parameterList>" << endl;
    }

    void compileStatements() {
        ofs_ << "<statements>" << endl;
        while(true) {
            if(!lookaheadKeyword({"let", "if", "while", "do", "return"})) {
                break;
            }
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
        assertWriteToken(TokenType::IDENTIFIER);
        // ('[' expression ']')?
        if(lookaheadSymbol({"["})) {
            assertWriteSymbol({"["});
            compileExpression();
            assertWriteSymbol({"]"});
        }
        assertWriteSymbol({"="});
        compileExpression();
        assertWriteSymbol({";"});
        ofs_ << "</letStatement>" << endl;
    }

    void compileIf() {
        ofs_ << "<ifStatement>" << endl;
        assertWriteKeyword({"if"});
        assertWriteSymbol({"("});
        compileExpression();
        assertWriteSymbol({")"});
        assertWriteSymbol({"{"});
        compileStatements();
        assertWriteSymbol({"}"});
        // ('else' '{' statements '}')?
        if(lookaheadKeyword({"else"})) {
            assertWriteKeyword({"else"});
            assertWriteSymbol({"{"});
            compileStatements();
            assertWriteSymbol({"}"});
        }
        ofs_ << "</ifStatement>" << endl;
    }

    void compileWhile() {
        ofs_ << "<whileStatement>" << endl;
        assertWriteKeyword({"while"});
        assertWriteSymbol({"("});
        compileExpression();
        assertWriteSymbol({")"});
        assertWriteSymbol({"{"});
        compileStatements();
        assertWriteSymbol({"}"});
        ofs_ << "</whileStatement>" << endl;
    }

    void compileDo() {
        ofs_ << "<doStatement>" << endl;
        assertWriteKeyword({"do"});
        compileSubroutineCall();
        assertWriteSymbol({";"});
        ofs_ << "</doStatement>" << endl;
    }

    void compileReturn() {
        ofs_ << "<returnStatement>" << endl;
        assertWriteKeyword({"return"});
        if(lookaheadExpression()) compileExpression();
        assertWriteSymbol({";"});
        ofs_ << "</returnStatement>" << endl;
    }

    void compileExpression() {
        ofs_ << "<expression>" << endl;
        compileTerm();
        while(true) {
            if(lookaheadSymbol(BINARY_OPERATORS)) {
                assertWriteSymbol(BINARY_OPERATORS);
                compileTerm();
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
        } else if(lookaheadToken(TokenType::STRING_CONST)) {
            assertWriteToken(TokenType::STRING_CONST);
        } else if(lookaheadKeyword({"true", "false", "null", "this"})) {
            assertWriteKeyword({"true", "false", "null", "this"});
        } else if(lookaheadToken(TokenType::IDENTIFIER)) {
            jack_tokenizer_.advance();
            if(lookaheadSymbol({"["})) {
                // varName '[' expression ']'
                jack_tokenizer_.retreat();
                assertWriteToken(TokenType::IDENTIFIER);
                assertWriteSymbol({"["});
                compileExpression();
                assertWriteSymbol({"]"});
            } else if(lookaheadSymbol({"(", "."})) {
                // subroutineCall
                jack_tokenizer_.retreat();
                compileSubroutineCall();
            } else {
                // varName
                jack_tokenizer_.retreat();
                assertWriteToken(TokenType::IDENTIFIER);
            }
        } else if(lookaheadSymbol({"("})) {
            // '(' expression ')'
            assertWriteSymbol({"("});
            compileExpression();
            assertWriteSymbol({")"});
        } else if(lookaheadSymbol(UNARY_OPERATORS)) {
            // unaryOp term
            assertWriteSymbol(UNARY_OPERATORS);
            compileTerm();
        }
        ofs_ << "</term>" << endl;
    }

    void compileSubroutineCall() {
        assertWriteToken(TokenType::IDENTIFIER);
        if(lookaheadSymbol({"."})) {
            assertWriteSymbol({"."});
            assertWriteToken(TokenType::IDENTIFIER);
        }
        assertWriteSymbol({"("});
        compileExpressionList();
        assertWriteSymbol({")"});
    }
       
    void compileExpressionList() {
        ofs_ << "<expressionList>" << endl;
        if(lookaheadExpression()) {
            compileExpression();
            while(true) {
                if(lookaheadSymbol({","})) {
                    assertWriteSymbol({","});
                    compileExpression();
                } else {
                    break;
                }
            }
        }
        ofs_ << "</expressionList>" << endl;
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

    void assertWriteToken(TokenType expected_token_type) {
        jack_tokenizer_.advance();
        if(jack_tokenizer_.tokenType() != expected_token_type) {
            compileError();
        } else {
            jack_tokenizer_.writeToken(ofs_, expected_token_type);
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

    void compileError() {
        // TODO: be more valuable message
        cout << "compilation failed" << endl;
    }
};
