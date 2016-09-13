%skeleton "lalr1.cc"
%defines
%locations
%define api.token.constructor
%define api.value.type variant
%define api.token.prefix {T_}
%define parse.error verbose

%code requires {
#include <string>
#include <vector>
#include <map>
#include <memory>

#include "location.hh"
#include "fiffiscript.hh"
#include "util.hh"

#define YY_DECL yy::parser::symbol_type yylex()
}

%code {
YY_DECL;
}

%parse-param { fiffiscript::Program& program }

%token  <long long>     INT_LITERAL
%token  <double>        FLOAT_LITERAL
%token  <std::string>   STRING_LITERAL
%token  <std::string>   IDENTIFIER
%token                  DEF NATIVE LEFT_PAREN RIGHT_PAREN LEFT_BRACE RIGHT_BRACE EQUALS
%token                  COMMA INT LONG SHORT FLOAT DOUBLE STRING VOID
%token                  EOF 0

%start program
%type   <std::vector<std::string>> param_list param_list1
%type   <std::vector<std::shared_ptr<fiffiscript::Expression>>> body
%type   <std::shared_ptr<fiffiscript::Expression>> expression primary_expression
%type   <std::vector<std::shared_ptr<fiffiscript::Expression>>> expression_list expression_list1
%type   <fiffiscript::NativeFunction::Type> type
%type   <std::vector<fiffiscript::NativeFunction::Type>> type_list type_list1
%type   <std::string> library_opt
%%

program:
                // nothing
        |       program definition
;

definition:
    DEF IDENTIFIER LEFT_PAREN param_list RIGHT_PAREN LEFT_BRACE body RIGHT_BRACE {
        auto f = std::make_shared<fiffiscript::RegularFunction>($2, $4, $7);
        auto exp = std::make_shared<fiffiscript::Constant>(f);
        program.add_definition($2, exp);
    } |
    DEF NATIVE library_opt type IDENTIFIER LEFT_PAREN type_list RIGHT_PAREN {
        auto f = std::make_shared<fiffiscript::NativeFunction>($3, $5, $4, $7);
        auto exp = std::make_shared<fiffiscript::Constant>(f);
        program.add_definition($5, exp);
    } |
    DEF IDENTIFIER EQUALS expression {
        program.add_definition($2, $4);
    }
;

param_list:
    {} |
    param_list1 { $$ = std::move($1); }

param_list1:
    IDENTIFIER {
        $$.push_back($1);
    } |
    param_list1 COMMA IDENTIFIER {
        $$ = std::move($1);
        $$.push_back($3);
    }
;

body:
    {} |
    body expression {
        $$ = std::move($1);
        $$.push_back($2);
    }
;

library_opt:
    { $$ = ""; } |
    LEFT_PAREN STRING_LITERAL RIGHT_PAREN  { $$ = std::move($2); }
;

type:
    SHORT { $$ = fiffiscript::NativeFunction::short_type; } |
    INT { $$ = fiffiscript::NativeFunction::int_type; } |
    LONG { $$ = fiffiscript::NativeFunction::long_type; } |
    LONG LONG { $$ = fiffiscript::NativeFunction::long_long_type; } |
    FLOAT { $$ = fiffiscript::NativeFunction::float_type; } |
    DOUBLE { $$ = fiffiscript::NativeFunction::double_type; } |
    STRING { $$ = fiffiscript::NativeFunction::string_type; } |
    VOID { $$ = fiffiscript::NativeFunction::void_type; }
;

type_list:
    {} |
    type_list1 { $$ = std::move($1); }

type_list1:
    type {
        $$.push_back($1);
    } |
    type_list1 COMMA type {
        $$ = std::move($1);
        $$.push_back($3);
    }
;

expression:
    primary_expression { $$ = $1; } |
    primary_expression LEFT_PAREN expression_list RIGHT_PAREN {
        $$ = std::make_shared<fiffiscript::FunctionCall>($1, $3);
    }
;

primary_expression:
    INT_LITERAL {
        auto val = std::make_shared<fiffiscript::IntValue>($1);
        $$ = std::make_shared<fiffiscript::Constant>(val);
    } |
    FLOAT_LITERAL {
        auto val = std::make_shared<fiffiscript::FloatValue>($1);
        $$ = std::make_shared<fiffiscript::Constant>(val);
    } |
    STRING_LITERAL {
        auto val = std::make_shared<fiffiscript::StringValue>($1);
        $$ = std::make_shared<fiffiscript::Constant>(val);
    } |
    IDENTIFIER {
        $$ = std::make_shared<fiffiscript::Variable>($1);
    } |
    LEFT_PAREN expression RIGHT_PAREN {
        $$ = $2;
    }
;

expression_list:
    {} |
    expression_list1 { $$ = std::move($1); }

expression_list1:
    expression {
        $$.push_back($1);
    } |
    expression_list1 COMMA expression {
        $$ = std::move($1);
        $$.push_back($3);
    }
;

%%

void yy::parser::error (const location& loc, const std::string& message)
{
    util::error(loc, message);
}
