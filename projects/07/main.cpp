#include <filesystem>
#include <fstream>
#include <iostream>

#include "./CodeWriter.hpp"
#include "./Constant.hpp"
#include "./Parser.hpp"

using namespace std;

void parseFile(const string& read_file, CodeWriter& code_writer) {
    // we parse .vm only
    if(read_file.find(".vm") == string::npos) return;

    Parser parser(read_file);
    int start = int(read_file.find_last_of('/') + 1);
    int end = int(read_file.find(".vm"));
    string read_file_name = read_file.substr(start, end - start);
    code_writer.setFileName(read_file_name);

    while(parser.hasMoreCommands()) {
        parser.advance();
        if(parser.isComment()) continue;
        CommandType command_type = parser.commandType();
        switch(command_type) {
            case CommandType::C_ARITHMETIC:
                code_writer.writeArithmetic(parser.arg1());
                break;
            case CommandType::C_PUSH:
                code_writer.writePushPop(CommandType::C_PUSH, parser.arg1(), parser.arg2());
                break;
            case CommandType::C_POP:
                code_writer.writePushPop(CommandType::C_POP, parser.arg1(), parser.arg2());
                break;
            default:
                break;
        }
    }
}

int main(int argc, char **argv) {
    if(argc < 2) {
        cout << "Usage: ./a.out {directory or file}" << endl;
        return 0;
    }

    CodeWriter code_writer("prog.asm");

    if(filesystem::is_directory(argv[1])) {
        for(const auto& file : filesystem::recursive_directory_iterator(argv[1])) { // c++17
            string read_file = file.path();
            parseFile(read_file, code_writer);
        }
    } else {
        string read_file = string(argv[1]);
        parseFile(read_file, code_writer);
    }
}
