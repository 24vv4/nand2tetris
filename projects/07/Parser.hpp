#pragma once
#include <fstream>
#include <iostream>
#include <vector>

#include "./Constant.hpp"

using namespace std;

class Parser {
public:
    Parser(const string& file_name) {
        ifstream ifs(file_name);
        if(!ifs) {
            cout << "open read file failed." << endl;
            return;
        }
        ifs_ = move(ifs);
    }

    bool hasMoreCommands() { return !ifs_.eof(); }

    void advance() {
        string line;
        getline(ifs_, line);
        vector<string> commands = parse(line);
        if(commands[0].empty()) {
            is_comment_ = true;
            return;
        }
        setCommandType(commands[0]);
        setArgs(commands);
        is_comment_ = false;
    }

    bool isComment() { return is_comment_; }

    CommandType commandType() { return command_type_; }

    const string& arg1() { return arg1_; }
    const int& arg2() { return arg2_; }

private:
    ifstream ifs_;
    bool is_comment_;
    string arg1_;
    int arg2_;
    CommandType command_type_;

    vector<string> parse(string &line) {
        trim(line);
        return split(line);
    }

    void trim(string &s) {
        if(s.find(COMMENT) != string::npos) {
            s.erase(s.find(COMMENT));
        }
        s.erase(s.find_last_not_of(CRSPACE) + 1);
        s.erase(0, s.find_first_not_of(CRSPACE));
    }

    vector<string> split(string &s) {
        char separator = ' ';
        int separator_length = 1;
        vector<string> commands;
        int offset = 0;
        while(true) {
            auto pos = s.find(separator, offset);
            if (pos == string::npos) {
                commands.push_back(s.substr(offset));
                break;
            }
            commands.push_back(s.substr(offset, pos - offset));
            offset = pos + separator_length;
        }
        return commands;
    }

    void setCommandType(const string& command) {
        if(command == "push") command_type_ = CommandType::C_PUSH;
        else if(command == "pop") command_type_ = CommandType::C_POP;
        else command_type_ = CommandType::C_ARITHMETIC;
    }

    void setArgs(const vector<string>& commands) {
        if(command_type_ == CommandType::C_ARITHMETIC) {
            arg1_ = commands[0];
        } else {
            arg1_ = commands[1];
            arg2_ = atoi(commands[2].c_str());
        }
    }
};
