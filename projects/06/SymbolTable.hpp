#include <map>
#include <string>

using namespace std;

class SymbolTable {
public:
    SymbolTable() {
        symbol_table_["SP"] = 0;
        symbol_table_["LCL"] = 1;
        symbol_table_["ARG"] = 2;
        symbol_table_["THIS"] = 3;
        symbol_table_["THAT"] = 4;
        for(int i = 0; i < 16; i++) {
            string key = "R" + to_string(i);
            symbol_table_[key] = i;
        }
        symbol_table_["SCREEN"] = 16384;
        symbol_table_["KBD"] = 24576;
    }

    void addEntry(const string &symbol, int address) {
        if(contains(symbol)) return;
        symbol_table_[symbol] = address;
    }

    bool contains(const string &symbol) {
        return symbol_table_.count(symbol) > 0;
    }

    int getAddress(const string &symbol) {
        return symbol_table_[symbol];
    }


private:
    map<string, int> symbol_table_;
};
