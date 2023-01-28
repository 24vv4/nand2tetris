#include "./Parser.hpp"

using namespace std;

int main(int argc, char **argv) {
    if(argc < 2) {
        cout << "Usage: ./a.out hoge.asm" << endl;
        return 0;
    }
    // init
    Parser parser(argv[1]);

    // first path
    parser.parseSymbol();
    parser.clearIfs();

    // second path
    parser.parseAll();
}
