#ifndef AST_HH
#define AST_HH

#include <string>
#include <vector>
#include <map>
#include <memory>
#include <ffi.h>

#include "util.hh"

namespace fiffiscript {
    class Value;

    class Environment {
        typedef std::map<std::string, std::shared_ptr<Value>> Scope;
        std::vector<Scope> scopes;
    public:
        Environment() {
            push_scope();
        }

        void push_scope() {
            scopes.emplace_back();
        }

        void pop_scope() {
            scopes.pop_back();
        }

        std::shared_ptr<Value>& operator[](const std::string& name);
    };

    class AstNode {
    protected:
        yy::location loc;

        AstNode(const yy::location& loc) : loc(loc) {}

        template<typename ...T>
        [[noreturn]] void error(const T&... args)
        {
            util::error(loc, args...);
        }

    public:
        virtual const yy::location& location() {
            return loc;
        }
    };


    class Value {
    public:
        virtual short to_short(const yy::location&) const = 0;
        virtual int to_int(const yy::location&) const = 0;
        virtual long to_long(const yy::location&) const = 0;
        virtual long long to_long_long(const yy::location&) const = 0;
        virtual float to_float(const yy::location&) const = 0;
        virtual double to_double(const yy::location&) const = 0;
        virtual std::string to_string(const yy::location&) const = 0;
        virtual std::string type_name() const = 0;
        virtual std::shared_ptr<Value> call(const yy::location& loc,
                                            const std::vector<std::shared_ptr<Value>>&,
                                            Environment&) {
            util::error(loc, "Tried to use value of type ", type_name(), " as a function.");
        }
        virtual ~Value() {}
    };

    class IntValue : public Value {
        long long value;
    public:
        IntValue(long long value) : value(value) {}

        virtual short to_short(const yy::location&) const {
            return value;
        }
        virtual int to_int(const yy::location&) const {
            return value;
        }
        virtual long to_long(const yy::location&) const {
            return value;
        }
        virtual long long to_long_long(const yy::location&) const {
            return value;
        }
        virtual float to_float(const yy::location&) const {
            return value;
        }
        virtual double to_double(const yy::location&) const {
            return value;
        }
        virtual std::string to_string(const yy::location&) const {
            return std::to_string(value);
        }
        virtual std::string type_name() const {
            return "int";
        }
    };

    class FloatValue : public Value {
        double value;
    public:
        FloatValue(double value) : value(value) {}

        virtual short to_short(const yy::location&) const {
            return value;
        }
        virtual int to_int(const yy::location&) const {
            return value;
        }
        virtual long to_long(const yy::location&) const {
            return value;
        }
        virtual long long to_long_long(const yy::location&) const {
            return value;
        }
        virtual float to_float(const yy::location&) const {
            return value;
        }
        virtual double to_double(const yy::location&) const {
            return value;
        }
        virtual std::string to_string(const yy::location&) const {
            return std::to_string(value);
        }
        virtual std::string type_name() const {
            return "float";
        }
    };

    class StringValue : public Value {
        std::string value;
    public:
        StringValue(const std::string& value) : value(value) {}

        [[noreturn]] void not_supported(const yy::location& loc, const std::string& type_name) const {
            util::error(loc, "Automatic conversion from strings to ", type_name, " not supported");
        }

        virtual short to_short(const yy::location& loc) const {
            not_supported(loc, "short");
        }
        virtual int to_int(const yy::location& loc) const {
            not_supported(loc, "int");
        }
        virtual long to_long(const yy::location& loc) const {
            not_supported(loc, "long");
        }
        virtual long long to_long_long(const yy::location& loc) const {
            not_supported(loc, "long long");
        }
        virtual float to_float(const yy::location& loc) const {
            not_supported(loc, "float");
        }
        virtual double to_double(const yy::location& loc) const {
            not_supported(loc, "double");
        }
        virtual std::string to_string(const yy::location&) const {
            return value;
        }
        virtual std::string type_name() const {
            return "string";
        }
    };

    class Function : public Value, public AstNode {
        [[noreturn]] void cast_error(const yy::location& loc, const std::string& type_name) const {
            util::error(loc, "Can't convert function to ", type_name);
        }

    protected:
        Function(const yy::location& loc) : AstNode(loc) {}

    public:
        virtual short to_short(const yy::location& loc) const {
            cast_error(loc, "short");
        }

        virtual int to_int(const yy::location& loc) const {
            cast_error(loc, "int");
        }

        virtual long to_long(const yy::location& loc) const {
            cast_error(loc, "long");
        }

        virtual long long to_long_long(const yy::location& loc) const {
            cast_error(loc, "long long");
        }

        virtual float to_float(const yy::location& loc) const {
            cast_error(loc, "float");
        }

        virtual double to_double(const yy::location& loc) const {
            cast_error(loc, "double");
        }

        virtual std::string to_string(const yy::location& loc) const {
            cast_error(loc, "string");
        }

        virtual std::string type_name() const {
            return "function";
        }
    };

    class NativeFunction : public Function {
    public:
        typedef ffi_type* Type;

    private:
        std::string library;
        std::string name;
        Type return_type;
        std::vector<Type> argument_types;
        ffi_cif cif;
        void* library_handle;
        void* function_handle;

    public:
        static const Type void_type;
        static const Type short_type;
        static const Type int_type;
        static const Type long_type;
        static const Type long_long_type;
        static const Type float_type;
        static const Type double_type;
        static const Type string_type;

        // Can't copy or re-assign native functions, so we don't need to worry about
        // what happens to the dlopen handles
        NativeFunction(const NativeFunction&) = delete;
        void operator=(const NativeFunction&) = delete;

        NativeFunction(const yy::location& loc,
                       const std::string& library,
                       const std::string& name,
                       Type return_type,
                       std::vector<Type>& argument_types);

        virtual std::shared_ptr<Value> call(const yy::location&,
                                            const std::vector<std::shared_ptr<Value>>&,
                                            Environment&);

        virtual ~NativeFunction();
    };

    class Expression : public AstNode {
    protected:
        Expression(const yy::location& loc) : AstNode(loc) {}

    public:
        virtual std::shared_ptr<Value> evaluate(Environment& environment) = 0;
        virtual ~Expression() {}
    };

    class Constant : public Expression {
        std::shared_ptr<Value> value;

    public:
        Constant(const yy::location& loc, std::shared_ptr<Value> value)
            : Expression(loc), value(value)
        {}

        virtual std::shared_ptr<Value> evaluate(Environment& environment) {
            return value;
        }
    };

    class Variable : public Expression {
        std::string name;
    public:
        Variable(const yy::location& loc, const std::string& name)
            : Expression(loc), name(name)
        {}

        virtual std::shared_ptr<Value> evaluate(Environment& environment);
    };

    class FunctionCall : public Expression {
        std::shared_ptr<Expression> function;
        std::vector<std::shared_ptr<Expression>> arguments;
    public:
        FunctionCall(const yy::location& loc,
                     std::shared_ptr<Expression> function,
                     const std::vector<std::shared_ptr<Expression>>& arguments)
            : Expression(loc), function(function), arguments(arguments)
        {}

        virtual std::shared_ptr<Value> evaluate(Environment& environment);
    };

    class RegularFunction : public Function {
        std::string name;
        std::vector<std::string> parameters;
        std::vector<std::shared_ptr<Expression>> body;
    public:
        RegularFunction(const yy::location& loc,
                        const std::string& name,
                        const std::vector<std::string>& parameters,
                        const std::vector<std::shared_ptr<Expression>>& body)
            : Function(loc), name(name), parameters(parameters), body(body)
        {}

        virtual std::shared_ptr<Value> call(const yy::location&,
                                            const std::vector<std::shared_ptr<Value>>&,
                                            Environment&);
    };

    struct Definition {
        std::string name;
        std::shared_ptr<Expression> body;
    };

    class Program : public AstNode {
        std::vector<Definition> definitions;
    public:
        Program(const yy::location& loc, const std::vector<Definition>& definitions)
            : AstNode(loc), definitions(definitions)
        {}

        void run();
    };
}

#endif
