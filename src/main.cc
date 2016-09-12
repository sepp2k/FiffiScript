#include "parser.tab.hh"
#include "fiffiscript.hh"
#include "tokenizer.hh"

int main(int argc, char** argv) {
    if(argc < 2) {
        tokenizer::init_stdin();
    } else {
        tokenizer::init_file(argv[1]);
    }
    fiffiscript::Program program;
    yy::parser parser(program);
    parser.parse();
    if(argc >= 2) {
        tokenizer::close_file();
    }
    program.run();
    return 0;
}
