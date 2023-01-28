#pragma once
#include <fstream>
#include <iostream>
#include <set>
#include <vector>

#include "./Constant.hpp"

using namespace std;

class JackTokenizer {
public:
    JackTokenizer(string file_name) {
        ifstream ifs(file_name);
        if(!ifs) {
            cout << "open read file failed." << endl;
            return;
        }
        ifs_ = move(ifs);

        int end = int(file_name.find(".jack"));
        file_name.replace(end, 5, "T.xml");
        ofstream ofs(file_name);
        if(!ofs) {
            cout << "open write file failed." << endl;
            return;
        }
        ofs_ = move(ofs);

        keyword_set_.insert("class");
        keyword_set_.insert("constructor");
        keyword_set_.insert("function");
        keyword_set_.insert("method");
        keyword_set_.insert("field");
        keyword_set_.insert("static");
        keyword_set_.insert("var");
        keyword_set_.insert("int");
        keyword_set_.insert("char");
        keyword_set_.insert("boolean");
        keyword_set_.insert("void");
        keyword_set_.insert("true");
        keyword_set_.insert("false");
        keyword_set_.insert("null");
        keyword_set_.insert("this");
        keyword_set_.insert("let");
        keyword_set_.insert("do");
        keyword_set_.insert("if");
        keyword_set_.insert("else");
        keyword_set_.insert("while");
        keyword_set_.insert("return");

        symbol_set_.insert(" ");
        symbol_set_.insert("{");
        symbol_set_.insert("}");
        symbol_set_.insert("(");
        symbol_set_.insert(")");
        symbol_set_.insert("[");
        symbol_set_.insert("]");
        symbol_set_.insert(".");
        symbol_set_.insert(",");
        symbol_set_.insert(";");
        symbol_set_.insert("+");
        symbol_set_.insert("-");
        symbol_set_.insert("*");
        symbol_set_.insert("/");
        symbol_set_.insert("&");
        symbol_set_.insert("|");
        symbol_set_.insert("<");
        symbol_set_.insert(">");
        symbol_set_.insert("=");
        symbol_set_.insert("~");
    }

    void tokenize() {
        bool is_comment = false;
        while(!ifs_.eof()) {
            string line;
            getline(ifs_, line);
            if(line.find(COMMENT_START) != string::npos) {
                is_comment = true;
            }
            if(is_comment) {
                if(line.find(COMMENT_END) != string::npos) is_comment = false;
                continue;
            }
            vector<string> tmp_tokens = parse(line);
            if(tmp_tokens.empty()) continue;
            tokens_.insert(tokens_.end(), tmp_tokens.begin(), tmp_tokens.end());
        }
    }

    void outputTXml() {
        ofs_ << "<tokens>" << endl;
        while(hasMoreTokens()) {
            advance();
            TokenType token_type = tokenType();
            writeToken(ofs_, token_type);
        }
        ofs_ << "</tokens>" << endl;
        seen_idx_ = 0;
    }


    bool hasMoreTokens() {
        return seen_idx_ < tokens_.size();
    }

    void advance() {
        setToken(tokens_[seen_idx_++]);
    }

    void retreat() {
        setToken(tokens_[--seen_idx_]);
    }

    TokenType tokenType() { return token_type_; }

    string keyword() { return keyword_; }

    string symbol() {
        if(symbol_ == "<") return "&lt;";
        else if(symbol_ == ">") return "&gt;";
        else if(symbol_ == "&") return "&amp;";
        else return symbol_;
    }

    string identifier() { return identifier_; }

    int intVal() { return intVal_; }

    string stringVal() { return stringVal_.substr(1); }

    void writeToken(ofstream &ofs, TokenType token_type) {
        switch(token_type) {
            case TokenType::KEYWORD:
                writeKeyword(ofs);
                break;
            case TokenType::SYMBOL:
                writeSymbol(ofs);
                break;
            case TokenType::IDENTIFIER:
                writeIdentifier(ofs);
                break;
            case TokenType::INT_CONST:
                writeIntVal(ofs);
                break;
            case TokenType::STRING_CONST:
                writeStringVal(ofs);
                break;
            default:
                break;
        }
    }

    void writeKeyword(ofstream& ofs_) {
        ofs_ << "<keyword> " << keyword() << " </keyword>" << endl;
    }

    void writeSymbol(ofstream& ofs_) {
        ofs_ << "<symbol> " << symbol() << " </symbol>" << endl;
    }

    void writeIdentifier(ofstream& ofs_) {
        ofs_ << "<identifier> " << identifier() << " </identifier>" << endl;
    }

    void writeIntVal(ofstream& ofs_) {
        ofs_ << "<integerConstant> " << intVal() << " </integerConstant>" << endl;
    }

    void writeStringVal(ofstream &ofs_) {
        ofs_ << "<stringConstant> " << stringVal() << " </stringConstant>" << endl;
    }

private:
    set<string> keyword_set_;
    set<string> symbol_set_;
    ifstream ifs_;
    ofstream ofs_;
    int seen_idx_ = 0;
    vector<string> tokens_;
    
    TokenType token_type_;
    string keyword_;
    string symbol_;
    string identifier_;
    int intVal_;
    string stringVal_;

    vector<string> parse(string &line) {
        trim(line);
        return splitString(line);
    }

    void trim(string &s) {
        if(s.find(COMMENT_LINE) != string::npos) {
            s.erase(s.find(COMMENT_LINE));
        }
        s.erase(s.find_last_not_of(CRSPACE) + 1);
        s.erase(0, s.find_first_not_of(CRSPACE));
    }

    vector<string> splitString(string &s) {
        char separator = '"';
        int separator_length = 1;
        vector<string> tokens;
        int offset = 0;
        bool is_string = false;
        while(true) {
            auto pos = s.find(separator, offset);
            if (pos == string::npos) {
                string ts = s.substr(offset);
                vector<string> tmp = splitSymbol(ts);
                if(tmp.empty()) break;
                tokens.insert(tokens.end(), tmp.begin(), tmp.end());
                break;
            }
            if(is_string) {
                // append first character '"'
                // e.g. "string constant
                tokens.push_back(s.substr(offset-1, pos - offset + 1));
                is_string = false;
            } else {
                string ts = s.substr(offset, pos - offset);
                vector<string> tmp = splitSymbol(ts);
                if(tmp.empty()) break;
                tokens.insert(tokens.end(), tmp.begin(), tmp.end());
                is_string = true;
            }
            offset = pos + separator_length;
        }
        return tokens;
    }

    vector<string> splitSymbol(string &s) {
        int separator_length = 1;
        vector<string> tokens;
        int offset = 0;
        while(true) {
            int pos = INT_MAX;
            string separator;
            for(string symbol : symbol_set_) {
                if(s.find(symbol, offset) != string::npos) {
                    int symbol_pos = (int)s.find(symbol, offset);
                    if(symbol_pos < pos) {
                        pos = symbol_pos;
                        separator = symbol;
                    }
                }
            }
            if (pos == INT_MAX) break;
            if(pos > offset) tokens.push_back(s.substr(offset, pos - offset));
            if(separator != " " && separator != "") tokens.push_back(separator);
            offset = pos + separator_length;
        }
        return tokens;
    }

    void setToken(const string& token) {
        resetToken();
        if(keyword_set_.find(token) != keyword_set_.end()) {
            token_type_ = TokenType::KEYWORD;
            keyword_ = token;
            return;
        }
        if(symbol_set_.find(token) != symbol_set_.end()) {
            token_type_ = TokenType::SYMBOL;
            symbol_ = token;
            return;
        }
        if(isdigit(token[0])) {
            token_type_ = TokenType::INT_CONST;
            intVal_ = atoi(token.c_str());
            return;
        }
        if(token[0] == '"') {
            token_type_ = TokenType::STRING_CONST;
            stringVal_ = token;
            return;
        }
        token_type_ = TokenType::IDENTIFIER;
        identifier_ = token;
    }

    void resetToken() {
        keyword_ = "";
        symbol_ = "";
        identifier_ = "";
        intVal_ = 0;
        stringVal_ = "";
    }
};
