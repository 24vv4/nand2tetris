#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <bitset>

#include "./Code.hpp"
#include "./SymbolTable.hpp"

using namespace std;

enum class CommandType {
    A_COMMAND,
    C_COMMAND,
    L_COMMAND
};

const int VARIABLE_START_ADDRESS = 16;
const string COMMENT = "//";
const string CRSPACE = "\n\r ";

class Parser {
public:
    Parser(const char* file_name) {
        // input file
        string read_file = string(file_name);
        ifstream ifs(read_file);
        if(!ifs) {
            cout << "open read file failed." << endl;
        }
        ifs_ = move(ifs);

        // output file
        // hoge.asm -> hoge.hack
        string write_file = read_file.substr(0, read_file.find(".")) + ".hack";
        ofstream ofs(write_file);
        if(!ofs) {
            cout << "open write file failed." << endl;
        }
        ofs_ = move(ofs);
    }

    void parseSymbol() {
        int line = 0;
        while(hasMoreCommands()) {
            advance();
            if(command_.empty()) continue;
            CommandType c = commandType();
            switch (c) {
                case CommandType::L_COMMAND:
                    {
                        command_.erase(command_.begin());
                        command_.erase(command_.find(')'));
                        symbol_table_.addEntry(command_, line);
                        break;
                    }
                default:
                    line++;
                    break;
            }
        }
    }

    void parseAll() {
        int var_address = VARIABLE_START_ADDRESS;
        while(hasMoreCommands()) {
            advance();
            if(command_.empty()) continue;
            CommandType c = commandType();
            switch (c) {
                case CommandType::A_COMMAND:
                    {
                        int num = 0;
                        command_.erase(command_.begin());
                        if(isdigit(command_[0])) {
                            num = atoi(command_.c_str());
                        } else if(symbol_table_.contains(command_)) {
                            // label
                            num = symbol_table_.getAddress(command_);
                        } else {
                            // variable
                            num = var_address;
                            symbol_table_.addEntry(command_, var_address);
                            var_address++;
                        }
                        stringstream ss;
                        ss << bitset<16>(num);
                        ofs_ << ss.str() << endl;
                        break;
                    }
                case CommandType::C_COMMAND:
                    ofs_ << "111";
                    {
                        // comp;jump
                        if(command_.find(';') != string::npos) {
                            int pos = command_.find(';');
                            string left = command_.substr(0, pos);
                            string right = command_.substr(pos+1);
                            ofs_ << code_.comp(left);
                            ofs_ << "000"; // no dest
                            ofs_ << code_.jump(right) << endl;
                        // dest=comp
                        } else {
                            int pos = command_.find('=');
                            string left = command_.substr(0, pos);
                            string right = command_.substr(pos+1);
                            ofs_ << code_.comp(right);
                            ofs_ << code_.dest(left);
                            ofs_ << "000" << endl; // no jump
                        }
                    }
                    break;
                case CommandType::L_COMMAND:
                    break;
            }
        }
    }

    bool hasMoreCommands() {
        return !ifs_.eof();
    }

    void advance() {
        getline(ifs_, command_);
        trim(command_);
    }

    void trim(string &s) {
        if(s.find(COMMENT) != string::npos) {
            s.erase(s.find(COMMENT));
        }
        s.erase(s.find_last_not_of(CRSPACE) + 1);
        s.erase(0, s.find_first_not_of(CRSPACE));
    }

    CommandType commandType() {
        if(command_[0] == '@') return CommandType::A_COMMAND;
        if(command_[0] == '(') return CommandType::L_COMMAND;
        return CommandType::C_COMMAND;
    }

    void clearIfs() {
        ifs_.clear();
        ifs_.seekg(0, std::ios::beg);
    }

private:
    ifstream ifs_;
    ofstream ofs_;
    string command_ = "";
    Code code_;
    SymbolTable symbol_table_;
};
