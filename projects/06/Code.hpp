#include <map>
#include <string>

using namespace std;

class Code {

public:
    Code() {
        comp_table["0"]   = "101010";
        comp_table["1"]   = "111111";
        comp_table["-1"]  = "111010";
        comp_table["D"]   = "001100";
        comp_table["A"]   = "110000";
        comp_table["M"]   = "110000";
        comp_table["!D"]  = "001101";
        comp_table["!A"]  = "110001";
        comp_table["!M"]  = "110001";
        comp_table["-D"]  = "001111";
        comp_table["-A"]  = "110011";
        comp_table["-M"]  = "110011";
        comp_table["D+1"] = "011111";
        comp_table["A+1"] = "110111";
        comp_table["M+1"] = "110111";
        comp_table["D-1"] = "001110";
        comp_table["A-1"] = "110010";
        comp_table["M-1"] = "110010";
        comp_table["D+A"] = "000010";
        comp_table["D+M"] = "000010";
        comp_table["D-A"] = "010011";
        comp_table["D-M"] = "010011";
        comp_table["A-D"] = "000111";
        comp_table["M-D"] = "000111";
        comp_table["D&A"] = "000000";
        comp_table["D&M"] = "000000";
        comp_table["D|A"] = "010101";
        comp_table["D|M"] = "010101";

        jump_table["JGT"]  = "001";
        jump_table["JEQ"]  = "010";
        jump_table["JGE"]  = "011";
        jump_table["JLT"]  = "100";
        jump_table["JNE"]  = "101";
        jump_table["JLE"]  = "110";
        jump_table["JMP"]  = "111";
    };

    string dest(const string &s) {
        string res;
        // d1 ~ d3
        if(s.find('A') != string::npos) res += "1";
        else res += "0";
        if(s.find('D') != string::npos) res += "1";
        else res += "0";
        if(s.find('M') != string::npos) res += "1";
        else res += "0";
        return res;
    }

    string comp(const string &s) {
        string res;
        // a
        if(s.find('M') != string::npos) res += "1";
        else res += "0";
        // c1 ~ c6
        res += comp_table[s];
        return res;
    }

    string jump(const string &s) {
        return jump_table[s];
    }


private:
    // key : comp
    // val : c1c2c3c4c5c6
    map<string, string> comp_table;

    // key : jump
    // val : j1j2j3
    map<string, string> jump_table;
};
