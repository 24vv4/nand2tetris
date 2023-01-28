#pragma once

#include <map>

#include "./Constant.hpp"

using namespace std;

class Symbol {
public:
    string type_;
    IDENTIFIER_KIND kind_;
    int index_;
};

class SymbolTable {
public:
    SymbolTable() {}

    void startSubroutine(string func) {
        subroutine_.clear();
        arg_count_ = 0;
        var_count_ = 0;
        // instance is pushed at arg 0
        if(func == "method") arg_count_++;
    }

    void define(string name, string type, IDENTIFIER_KIND kind) {
        switch(kind) {
            case IDENTIFIER_KIND::STATIC: {
                class_[name] = Symbol{.type_=type, .kind_=kind, .index_=static_count_++};
                break;
            }
            case IDENTIFIER_KIND::FIELD: {
                class_[name] = Symbol{.type_=type, .kind_=kind, .index_=field_count_++};
                break;
            }
            case IDENTIFIER_KIND::ARG: {
                subroutine_[name] = Symbol{.type_=type, .kind_=kind, .index_=arg_count_++};
                break;
            }
            case IDENTIFIER_KIND::VAR: {
                subroutine_[name] = Symbol{.type_=type, .kind_=kind, .index_=var_count_++};
                break;
            }
            default: ;
        }
    }

    int varCount(IDENTIFIER_KIND kind) {
        switch(kind) {
            case IDENTIFIER_KIND::STATIC: return static_count_;
            case IDENTIFIER_KIND::FIELD: return field_count_;
            case IDENTIFIER_KIND::ARG: return arg_count_;
            case IDENTIFIER_KIND::VAR: return var_count_;
            default: return -1;
        }
    }

    IDENTIFIER_KIND kindOf(string name) {
        if(subroutine_.find(name) != subroutine_.end()) {
            return subroutine_[name].kind_;
        } else if(class_.find(name) != class_.end()) {
            return class_[name].kind_;
        } else {
            return IDENTIFIER_KIND::NONE;
        }
    }

    string typeOf(string name) {
        if(subroutine_.find(name) != subroutine_.end()) {
            return subroutine_[name].type_;
        } else if(class_.find(name) != class_.end()) {
            return class_[name].type_;
        } else {
            return "";
        }
    }

    int indexOf(string name) {
        if(subroutine_.find(name) != subroutine_.end()) {
            return subroutine_[name].index_;
        } else if(class_.find(name) != class_.end()) {
            return class_[name].index_;
        } else {
            return -1;
        }
    }
 

private:
    map<string, Symbol> class_;
    map<string, Symbol> subroutine_;
    int static_count_ = 0;
    int field_count_ = 0;
    int arg_count_ = 0;
    int var_count_ = 0;
};
