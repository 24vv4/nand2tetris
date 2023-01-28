#include <filesystem>
#include <fstream>
#include <iostream>

#include "./CompilationEngine.hpp"
#include "./JackTokenizer.hpp"
#include "./Constant.hpp"

using namespace std;

void parseFile(string read_file) {
    // we parse .jack only
    if(read_file.find(".jack") == string::npos) return;
    JackTokenizer tokenizer(read_file);
    tokenizer.tokenize();
    tokenizer.outputTXml();

    CompilationEngine compilation_engine(read_file, tokenizer);
    compilation_engine.compileClass();
}


int main(int argc, char **argv) {
    if(argc < 2) {
        cout << "Usage: ./a.out {directory or file}" << endl;
        return 0;
    }

    if(filesystem::is_directory(argv[1])) {
        for(const auto& file : filesystem::recursive_directory_iterator(argv[1])) { // c++17
            string read_file = file.path();
            parseFile(read_file);
        }
    } else {
        string read_file = string(argv[1]);
        parseFile(read_file);
    }
}
