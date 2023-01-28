#pragma once

#include <fstream>
#include <iostream>

using namespace std;

class VMWriter {
public:
    VMWriter(string file_name) {
        int end = int(file_name.find(".jack"));
        file_name.replace(end, 5, ".vm");
        ofstream ofs(file_name);
        cout << file_name << endl;
        if(!ofs) {
            cout << "open write .vm file failed." << endl;
            return;
        }
        ofs_ = move(ofs);
    }

    void writePush(MEMORY_SEGMENT segment, int index) {
        switch(segment) {
            case MEMORY_SEGMENT::CONST: {
                ofs_ << "push constant " << index << endl;
                break;
            }
            case MEMORY_SEGMENT::ARG: {
                ofs_ << "push argument " << index << endl;
                break;
            }
            case MEMORY_SEGMENT::LOCAL: {
                ofs_ << "push local " << index << endl;
                break;
            }
            case MEMORY_SEGMENT::STATIC: {
                ofs_ << "push static " << index << endl;
                break;
            }
            case MEMORY_SEGMENT::THIS: {
                ofs_ << "push this " << index << endl;
                break;
            }
            case MEMORY_SEGMENT::THAT: {
                ofs_ << "push that " << index << endl;
                break;
            }
            case MEMORY_SEGMENT::POINTER: {
                ofs_ << "push pointer " << index << endl;
                break;
            }
            case MEMORY_SEGMENT::TEMP: {
                ofs_ << "push temp " << index << endl;
                break;
            }
        }
    }

    void writePop(MEMORY_SEGMENT segment, int index) {
         switch(segment) {
            case MEMORY_SEGMENT::CONST: {
                ofs_ << "pop constant " << index << endl;
                break;
            }
            case MEMORY_SEGMENT::ARG: {
                ofs_ << "pop argument " << index << endl;
                break;
            }
            case MEMORY_SEGMENT::LOCAL: {
                ofs_ << "pop local " << index << endl;
                break;
            }
            case MEMORY_SEGMENT::STATIC: {
                ofs_ << "pop static " << index << endl;
                break;
            }
            case MEMORY_SEGMENT::THIS: {
                ofs_ << "pop this " << index << endl;
                break;
            }
            case MEMORY_SEGMENT::THAT: {
                ofs_ << "pop that " << index << endl;
                break;
            }
            case MEMORY_SEGMENT::POINTER: {
                ofs_ << "pop pointer " << index << endl;
                break;
            }
            case MEMORY_SEGMENT::TEMP: {
                ofs_ << "pop temp " << index << endl;
                break;
            }
        }
    }

    void writeArithmetic(COMMAND command) {
        if(command == COMMAND::ADD) ofs_ << "add" << endl;
        if(command == COMMAND::SUB) ofs_ << "sub" << endl;
        if(command == COMMAND::NEG) ofs_ << "neg" << endl;
        if(command == COMMAND::EQ)  ofs_ << "eq" << endl;
        if(command == COMMAND::GT)  ofs_ << "gt" << endl;
        if(command == COMMAND::LT)  ofs_ << "lt" << endl;
        if(command == COMMAND::AND) ofs_ << "and" << endl;
        if(command == COMMAND::OR)  ofs_ << "or" << endl;
        if(command == COMMAND::NOT) ofs_ << "not" << endl;
        if(command == COMMAND::MUL) writeCall("Math.multiply", 2);
        if(command == COMMAND::DIV) writeCall("Math.divide", 2);
    }

    void writeLabel(string label) {
        ofs_ << "label " << label << endl;
    }

    void writeGoto(string label) {
        ofs_ << "goto " << label << endl;
    }

    void writeIf(string label) {
        ofs_ << "if-goto " << label << endl;
    }

    void writeCall(string name, int n_args, bool class_method = false) {
        if(class_method) name = class_name_ + "." + name;
        ofs_ << "call " << name << " " << n_args << endl;
    }

    void writeFunction(string subroutine_name, int n_locals) {
        ofs_ << "function " << class_name_ << "." << subroutine_name << " " << n_locals << endl;
    }

    void writeReturn() {
        ofs_ << "return" << endl;
    }

    void close() {
    }

    void writeMemoryAlloc() {
        ofs_ << "call Memory.alloc 1" << endl;
    }

    void setClassName(string name) {
        class_name_ = name;
    }

    void setSubroutineName(string name) {
        subroutine_name_ = name;
    }

    int getLabel() {
        return label_++;
    }

private:
    ofstream ofs_;
    string class_name_;
    string subroutine_name_;
    int label_ = 0;
};
