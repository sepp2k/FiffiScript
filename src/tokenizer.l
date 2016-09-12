%{
#include <string>
#include <cstring>
#include <cstdio>
#include "parser.tab.hh"
#include "util.hh"
%}

%option noyywrap nounput batch noinput

%%

[ \t\r\n]+
"#".*
"("        return yy::parser::make_LEFT_PAREN();
")"        return yy::parser::make_RIGHT_PAREN();
"{"        return yy::parser::make_LEFT_BRACE();
"}"        return yy::parser::make_RIGHT_BRACE();
"="        return yy::parser::make_EQUALS();
","        return yy::parser::make_COMMA();
"def"      return yy::parser::make_DEF();
"native"   return yy::parser::make_NATIVE();
"short"    return yy::parser::make_SHORT();
"int"      return yy::parser::make_INT();
"long"     return yy::parser::make_LONG();
"float"    return yy::parser::make_FLOAT();
"double"   return yy::parser::make_DOUBLE();
"string"   return yy::parser::make_STRING();
"void"     return yy::parser::make_VOID();

[0-9]+     return yy::parser::make_INT_LITERAL(std::stoi(yytext));
[0-9]+\.[0-9]+ return yy::parser::make_FLOAT_LITERAL(std::stod(yytext));
[a-zA-Z][a-zA-Z_0-9]*  return yy::parser::make_IDENTIFIER(yytext);
["][^"]*["] return yy::parser::make_STRING_LITERAL(std::string(yytext + 1, std::strlen(yytext)-2));
<<EOF>>    return yy::parser::make_EOF();
.          util::error("Invalid character");

%%

namespace tokenizer {
    void init_stdin() {
        yyin = stdin;
    }

    void init_file(const char* filename) {
        yyin = std::fopen(filename, "r");
    }

    void close_file() {
        std::fclose(yyin);
    }
}
