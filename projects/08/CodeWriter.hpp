#pragma once
#include <fstream>
#include <iostream>

#include "./Constant.hpp"

using namespace std;

class CodeWriter {
public:
    CodeWriter(const string& write_file_name) {
        ofstream ofs(write_file_name);
        if(!ofs) {
            cout << "open write file failed." << endl;
            return;
        }
        ofs_ = move(ofs);
    }

    void setFileName(const string& read_file_name) {
        read_file_name_ = read_file_name;
    }

    void writeBootstrap() {
        // SP = 256
        setConstantToD("256");
        ofs_ << "@SP" << endl;
        ofs_ << "M=D" << endl;
        // call Sys.init
        writeCall("Sys.init", 0);
    }

    void writeArithmetic(const string& command) {
        if(command == "neg" || command == "not") writeUnaryCommand(command);
        else writeBinaryCommand(command);
    }

    void writePushPop(CommandType command_type, const string& segment, const int& index) {
        if(command_type == CommandType::C_POP) popD();
        if(segment == "constant") setConstantToD(to_string(index));
        else if(segment == "local" || segment == "argument" || segment == "this" || segment == "that") setBaseToA(segment, index);
        else if(segment == "pointer" || segment == "temp") setRegisterToA(segment, index);
        else if(segment == "static") setStaticToA(to_string(index));
        if(command_type == CommandType::C_POP) ofs_ << "M=D" << endl;
        else {
            if(segment != "constant") ofs_ << "D=M" << endl;
            pushD();
        }
    }

    void writeLabel(const string& label) {
        ofs_ << "(" + function_name_ + "$" + label + ")" << endl;
    }

    void writeGoto(const string& label) {
        ofs_ << "@" + function_name_ + "$" + label << endl;
        ofs_ << "0;JMP" << endl;
    }

    void writeIf(const string& label) {
        popD();
        ofs_ << "@" + function_name_ + "$" + label << endl;
        ofs_ << "D;JNE" << endl;
    }

    void writeFunction(const string& function_name, int num_args) {
        setFunctionName(function_name);
        ofs_ << "(" + function_name + ")" << endl;
        setConstantToD("0");
        for(int i = 0; i < num_args; i++) {
            pushD();
        }
    }

    void writeCall(const string& function_name, int num_args) {
        // push return-address
        string return_label = getLabel();
        ofs_ << "@" + return_label << endl;
        ofs_ << "D=A" << endl;
        pushD();
        // save segment
        for(auto segment : {"@LCL", "@ARG", "@THIS", "@THAT"}) {
            ofs_ << segment << endl;
            ofs_ << "D=M" << endl;
            pushD();
        }
        // LCL = SP
        copyRegister("@SP", "@LCL");
        // ARG = SP - 5 - num_args
        for(int i = 0; i < 5 + num_args; i++) ofs_ << "D=D-1" << endl;
        ofs_ << "@ARG" << endl;
        ofs_ << "M=D" << endl;
        // goto function_name
        ofs_ << "@" + function_name << endl;
        ofs_ << "0;JMP" << endl;
        // (return-address)
        ofs_ << "(" + return_label + ")" << endl;
    }

    void writeReturn() {
        // R13(FRAME) = LCL
        copyRegister("@LCL", "@R13");
        // R14(RET) = *(FRAME - 5)
        for(int i = 0; i < 5; i++) ofs_ << "D=D-1" << endl;
        copyRegister("A=D", "@R14");
        // *ARG = pop()
        writePushPop(CommandType::C_POP, "argument", 0);
        // SP = ARG + 1
        setBaseToA("argument", 1);
        ofs_ << "D=A" << endl;
        ofs_ << "@SP" << endl;
        ofs_ << "M=D" << endl;
        // restore segment
        for(auto segment : {"@THAT", "@THIS", "@ARG", "@LCL"}) {
            ofs_ << "@R13" << endl;
            ofs_ << "M=M-1" << endl;
            copyRegister("A=M", segment);
        }
        // goto RET
        ofs_ << "@R14" << endl;
        ofs_ << "A=M" << endl;
        ofs_ << "0;JMP" << endl;
    }


private:
    string read_file_name_;
    string function_name_;
    ofstream ofs_;
    int label_count_ = 0;

    string getLabel() {
        return "Label" + to_string(label_count_++);
    }

    void setFunctionName(const string& function_name) {
        function_name_ = function_name;
    }

    // neg, not
    void writeUnaryCommand(const string& command) {
        ofs_ << "@SP" << endl;
        ofs_ << "A=M-1" << endl;
        if(command == "neg") ofs_ << "M=-M" << endl;
        if(command == "not") ofs_ << "M=!M" << endl;
    }

    // add, sub, and, or, eq, lt, gt
    void writeBinaryCommand(const string& command) {
        // pop * 2
        // x => M, y => D
        popD();
        ofs_ << "@SP" << endl;
        ofs_ << "M=M-1" << endl;
        ofs_ << "A=M" << endl;
        if(command == "add" || command == "sub" || command == "and" || command == "or") {
            if(command == "add") ofs_ << "D=D+M" << endl;
            if(command == "sub") ofs_ << "D=M-D" << endl;
            if(command == "and") ofs_ << "D=D&M" << endl;
            if(command == "or") ofs_ << "D=D|M" << endl;
            pushD();
        }
        if(command == "eq" || command == "lt" || command == "gt") {
            ofs_ << "D=M-D" << endl;
            string l1 = getLabel(); // true
            string l2 = getLabel(); // end
            ofs_ << "@" + l1 << endl;
            if(command == "eq") ofs_ << "D;JEQ" << endl;
            if(command == "gt") ofs_ << "D;JGT" << endl;
            if(command == "lt") ofs_ << "D;JLT" << endl;
            // false
            ofs_ << "D=0" << endl;
            pushD();
            ofs_ << "@" + l2 << endl;
            ofs_ << "0;JMP" << endl;
            // true
            // (l1)
            ofs_ << "(" + l1 + ")" << endl;
            ofs_ << "D=-1" << endl;
            pushD();
            // (l2)
            ofs_ << "(" + l2 + ")" << endl;
        }
    }

    void pushD() {
        ofs_ << "@SP" << endl;
        ofs_ << "A=M" << endl;
        ofs_ << "M=D" << endl;
        ofs_ << "@SP" << endl;
        ofs_ << "M=M+1" << endl;
    }

    void popD() {
        ofs_ << "@SP" << endl;
        ofs_ << "AM=M-1" << endl;
        ofs_ << "D=M" << endl;
    }

    // constant number
    void setConstantToD(const string& index) {
        if(index[0] == '-') {
            ofs_ << "@" << index.substr(1) << endl;
            ofs_ << "D=-A" << endl;
        } else {
            ofs_ << "@" + index << endl;
            ofs_ << "D=A" << endl;
        }
    }

    // M[Register] + index
    void setBaseToA(const string& segment, const int& index) {
        if(segment == "local") ofs_ << "@LCL" << endl;
        if(segment == "argument") ofs_ << "@ARG" << endl;
        if(segment == "this") ofs_ << "@THIS" << endl;
        if(segment == "that") ofs_ << "@THAT" << endl;
        ofs_ << "A=M" << endl;
        for(int i = 0; i < index; i++) {
            ofs_ << "A=A+1" << endl;
        }
    }

    // Register + index
    void setRegisterToA(const string& segment, const int& index) {
        int register_index;
        if(segment == "pointer") register_index = POINTER_BASE + index;
        if(segment == "temp") register_index = TEMP_BASE + index;
        ofs_ << "@R" + to_string(register_index) << endl;
    }

    // static variable address
    void setStaticToA(const string& index) {
        ofs_ << "@" + read_file_name_ + "." + index << endl;
    }

    void copyRegister(const string& from, const string& to) {
        ofs_ << from << endl;
        ofs_ << "D=M" << endl;
        ofs_ << to << endl;
        ofs_ << "M=D" << endl;
    }
};
