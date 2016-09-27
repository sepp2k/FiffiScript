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

%parse-param { std::unique_ptr<fiffiscript::Program>& program }

%token  <long long>     INT_LITERAL
%token  <double>        FLOAT_LITERAL
%token  <std::string>   STRING_LITERAL
%token  <std::string>   IDENTIFIER
%token                  DEF NATIVE LEFT_PAREN RIGHT_PAREN LEFT_BRACE RIGHT_BRACE EQUALS
%token                  COMMA SEMI INT LONG SHORT FLOAT DOUBLE STRING VOID
%token                  EOF 0

%start program
%type   <std::vector<std::string>> param_list param_list1
%type   <std::vector<std::shared_ptr<fiffiscript::Expression>>> body
%type   <std::shared_ptr<fiffiscript::Expression>> expression primary_expression
%type   <std::vector<std::shared_ptr<fiffiscript::Expression>>> expression_list expression_list1
%type   <fiffiscript::NativeFunction::Type> type
%type   <std::vector<fiffiscript::NativeFunction::Type>> type_list type_list1
%type   <std::string> library_opt
%type   <std::vector<fiffiscript::Definition>> definitions
%type   <fiffiscript::Definition> definition
%%

program:
    definitions {
        program = std::make_unique<fiffiscript::Program>(@program, $definitions);
    }
;

definitions[result]:
    {} |
    definitions[previous] definition {
        $result = std::move($previous);
        $result.push_back($definition);
    }
;

definition:
    DEF IDENTIFIER LEFT_PAREN param_list RIGHT_PAREN LEFT_BRACE body RIGHT_BRACE {
        auto f = std::make_shared<fiffiscript::RegularFunction>(@definition,
                                                                $IDENTIFIER,
                                                                $param_list,
                                                                $body);
        auto exp = std::make_shared<fiffiscript::Constant>(@definition, f);
        $definition.name = $2;
        $definition.body = exp;
    } |
    DEF NATIVE library_opt type IDENTIFIER LEFT_PAREN type_list RIGHT_PAREN {
        auto f = std::make_shared<fiffiscript::NativeFunction>(@definition,
                                                               $library_opt,
                                                               $IDENTIFIER,
                                                               $type,
                                                               $type_list);
        auto exp = std::make_shared<fiffiscript::Constant>(@definition, f);
        $definition.name = $IDENTIFIER;
        $definition.body = exp;
    } |
    DEF IDENTIFIER EQUALS expression {
        $definition.name = $IDENTIFIER;
        $definition.body = $expression;
    }
;

param_list:
    {} |
    param_list1 { $param_list = std::move($param_list1); }

param_list1[result]:
    IDENTIFIER {
        $result.push_back($IDENTIFIER);
    } |
    param_list1[previous] COMMA IDENTIFIER {
        $result = std::move($previous);
        $result.push_back($IDENTIFIER);
    }
;

body[result]:
    {} |
    body[previous] expression SEMI {
        $result = std::move($previous);
        $result.push_back($expression);
    }
;

library_opt:
    { $library_opt = ""; } |
    LEFT_PAREN STRING_LITERAL RIGHT_PAREN  { $library_opt = std::move($STRING_LITERAL); }
;

type:
    SHORT { $type = fiffiscript::NativeFunction::short_type; } |
    INT { $type = fiffiscript::NativeFunction::int_type; } |
    LONG { $type = fiffiscript::NativeFunction::long_type; } |
    LONG LONG { $type = fiffiscript::NativeFunction::long_long_type; } |
    FLOAT { $type = fiffiscript::NativeFunction::float_type; } |
    DOUBLE { $type = fiffiscript::NativeFunction::double_type; } |
    STRING { $type = fiffiscript::NativeFunction::string_type; } |
    VOID { $type = fiffiscript::NativeFunction::void_type; }
;

type_list:
    {} |
    type_list1 { $type_list = std::move($type_list1); }

type_list1[result]:
    type {
        $result.push_back($type);
    } |
    type_list1[previous] COMMA type {
        $result = std::move($previous);
        $result.push_back($type);
    }
;

expression[result]:
    primary_expression { $result = $primary_expression; } |
    expression[f] LEFT_PAREN expression_list[args] RIGHT_PAREN {
        $result = std::make_shared<fiffiscript::FunctionCall>(@result, $f, $args);
    }
;

primary_expression[result]:
    INT_LITERAL {
        auto val = std::make_shared<fiffiscript::IntValue>($INT_LITERAL);
        $result = std::make_shared<fiffiscript::Constant>(@result, val);
    } |
    FLOAT_LITERAL {
        auto val = std::make_shared<fiffiscript::FloatValue>($FLOAT_LITERAL);
        $result = std::make_shared<fiffiscript::Constant>(@result, val);
    } |
    STRING_LITERAL {
        auto val = std::make_shared<fiffiscript::StringValue>($STRING_LITERAL);
        $result = std::make_shared<fiffiscript::Constant>(@result, val);
    } |
    IDENTIFIER {
        $result = std::make_shared<fiffiscript::Variable>(@result, $IDENTIFIER);
    } |
    LEFT_PAREN expression RIGHT_PAREN {
        $result = $expression;
    }
;

expression_list:
    {} |
    expression_list1 { $expression_list = std::move($expression_list1); }

expression_list1[result]:
    expression {
        $result.push_back($expression);
    } |
    expression_list1[previous] COMMA expression {
        $result = std::move($previous);
        $result.push_back($expression);
    }
;

%%

void yy::parser::error (const location& loc, const std::string& message)
{
    util::error(loc, message);
}
