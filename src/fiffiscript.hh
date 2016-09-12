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


    class Value {
    public:
        virtual short to_short() const = 0;
        virtual int to_int() const = 0;
        virtual long to_long() const = 0;
        virtual long long to_long_long() const = 0;
        virtual float to_float() const = 0;
        virtual double to_double() const = 0;
        virtual std::string to_string() const = 0;
        virtual std::shared_ptr<Value> call(const std::vector<std::shared_ptr<Value>>&,
                                            Environment&) {
            util::error("Tried to call non-function.");
        }
        virtual ~Value() {}
    };

    class IntValue : public Value {
        long long value;
    public:
        IntValue(long long value) : value(value) {}

        virtual short to_short() const {
            return value;
        }
        virtual int to_int() const {
            return value;
        }
        virtual long to_long() const {
            return value;
        }
        virtual long long to_long_long() const {
            return value;
        }
        virtual float to_float() const {
            return value;
        }
        virtual double to_double() const {
            return value;
        }
        virtual std::string to_string() const {
            return std::to_string(value);
        }
    };

    class FloatValue : public Value {
        double value;
    public:
        FloatValue(double value) : value(value) {}

        virtual short to_short() const {
            return value;
        }
        virtual int to_int() const {
            return value;
        }
        virtual long to_long() const {
            return value;
        }
        virtual long long to_long_long() const {
            return value;
        }
        virtual float to_float() const {
            return value;
        }
        virtual double to_double() const {
            return value;
        }
        virtual std::string to_string() const {
            return std::to_string(value);
        }
    };

    class StringValue : public Value {
        std::string value;
    public:
        StringValue(const std::string& value) : value(value) {}

        [[noreturn]] void not_supported() const {
            util::error("Automatic conversion from strings to numbers not supported");
        }

        virtual short to_short() const {
            not_supported();
        }
        virtual int to_int() const {
            not_supported();
        }
        virtual long to_long() const {
            not_supported();
        }
        virtual long long to_long_long() const {
            not_supported();
        }
        virtual float to_float() const {
            not_supported();
        }
        virtual double to_double() const {
            not_supported();
        }
        virtual std::string to_string() const {
            return value;
        }
    };

    class Function : public Value {
        [[noreturn]] void num_error() const {
            util::error("Can't use function as number");
        }
    public:
        virtual short to_short() const {
            num_error();
        }

        virtual int to_int() const {
            num_error();
        }

        virtual long to_long() const {
            num_error();
        }

        virtual long long to_long_long() const {
            num_error();
        }

        virtual float to_float() const {
            num_error();
        }

        virtual double to_double() const {
            num_error();
        }

        virtual std::string to_string() const {
            util::error("Can't use function as string");
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

        NativeFunction(const std::string& library,
                       const std::string& name,
                       Type return_type,
                       std::vector<Type>& argument_types);

        virtual std::shared_ptr<Value> call(const std::vector<std::shared_ptr<Value>>&,
                                            Environment&);

        virtual ~NativeFunction();
    };

    class Expression {
    public:
        virtual std::shared_ptr<Value> evaluate(Environment& environment) = 0;
        virtual ~Expression() {}
    };

    class Constant : public Expression {
        std::shared_ptr<Value> value;

    public:
        Constant(std::shared_ptr<Value> value) : value(value) {}

        virtual std::shared_ptr<Value> evaluate(Environment& environment) {
            return value;
        }
    };

    class Variable : public Expression {
        std::string name;
    public:
        Variable(const std::string& name) : name(name) {}
        virtual std::shared_ptr<Value> evaluate(Environment& environment);
    };

    class FunctionCall : public Expression {
        std::shared_ptr<Expression> function;
        std::vector<std::shared_ptr<Expression>> arguments;
    public:
        FunctionCall(std::shared_ptr<Expression> function,
                     const std::vector<std::shared_ptr<Expression>>& arguments)
            : function(function), arguments(arguments)
        {}

        virtual std::shared_ptr<Value> evaluate(Environment& environment);
    };

    class RegularFunction : public Function {
        std::vector<std::string> parameters;
        std::vector<std::shared_ptr<Expression>> body;
    public:
        RegularFunction(const std::vector<std::string>& parameters,
                        const std::vector<std::shared_ptr<Expression>>& body)
            : parameters(parameters), body(body)
        {}

        virtual std::shared_ptr<Value> call(const std::vector<std::shared_ptr<Value>>&,
                                            Environment&);
    };

    struct Definition {
        std::string name;
        std::shared_ptr<Expression> body;
    };

    class Program {
        std::vector<Definition> definitions;
    public:
        void add_definition(const std::string name, std::shared_ptr<Expression> value) {
            definitions.push_back({name, std::move(value)});
        }

        void run();
    };
}

#endif
