%{
#include <string>
#include <cstring>
#include <cstdio>
#include "parser.tab.hh"
#include "util.hh"

static yy::location loc;
%}

%option noyywrap nounput batch noinput

%{
    # define YY_USER_ACTION  loc.columns (yyleng);
%}
%%
%{
    loc.step ();
%}

[ \t\r]+   loc.step();
[\n]+      loc.lines(yyleng); loc.step();
"#".*      loc.step();
"("        return yy::parser::make_LEFT_PAREN(loc);
")"        return yy::parser::make_RIGHT_PAREN(loc);
"{"        return yy::parser::make_LEFT_BRACE(loc);
"}"        return yy::parser::make_RIGHT_BRACE(loc);
"="        return yy::parser::make_EQUALS(loc);
","        return yy::parser::make_COMMA(loc);
";"        return yy::parser::make_SEMI(loc);
"def"      return yy::parser::make_DEF(loc);
"native"   return yy::parser::make_NATIVE(loc);
"short"    return yy::parser::make_SHORT(loc);
"int"      return yy::parser::make_INT(loc);
"long"     return yy::parser::make_LONG(loc);
"float"    return yy::parser::make_FLOAT(loc);
"double"   return yy::parser::make_DOUBLE(loc);
"string"   return yy::parser::make_STRING(loc);
"void"     return yy::parser::make_VOID(loc);

[0-9]+     return yy::parser::make_INT_LITERAL(std::stoi(yytext), loc);
[0-9]+\.[0-9]+ return yy::parser::make_FLOAT_LITERAL(std::stod(yytext), loc);
[a-zA-Z][a-zA-Z_0-9]*  return yy::parser::make_IDENTIFIER(yytext, loc);
["][^"]*["] return yy::parser::make_STRING_LITERAL(std::string(yytext + 1, std::strlen(yytext)-2), loc);
<<EOF>>    return yy::parser::make_EOF(loc);
.          util::error(loc, "Invalid character '", yytext, "'");

%%

static std::string filename;

namespace tokenizer {
    void init_stdin() {
        yyin = stdin;
        filename = "(stdin)";
        loc.begin.filename = loc.end.filename = &filename;
    }

    void init_file(const char* name) {
        yyin = std::fopen(name, "r");
        filename = name;
        loc.begin.filename = loc.end.filename = &filename;
    }

    void close_file() {
        std::fclose(yyin);
    }
}
