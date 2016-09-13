#include <dlfcn.h>
#include <ffi.h>
#include <string>
#include <vector>
#include <map>
#include <cstring>

#include "fiffiscript.hh"
#include "util.hh"

namespace fiffiscript {
    std::shared_ptr<fiffiscript::Value>& Environment::operator[](const std::string& name) {
        for(auto iter = scopes.rbegin(); iter < scopes.rend(); iter++) {
            Scope& scope = *iter;
            if(scope[name]) return scope[name];
        }
        // If the name does not exist in the environment return a reference
        // into the topmost scope, so new entries are created in the most
        // current scope
        return (*scopes.rbegin())[name];
    }

    const NativeFunction::Type NativeFunction::void_type = &ffi_type_void;
    const NativeFunction::Type NativeFunction::short_type = &ffi_type_sshort;
    const NativeFunction::Type NativeFunction::int_type = &ffi_type_sint;
    const NativeFunction::Type NativeFunction::long_type = &ffi_type_slong;
    const NativeFunction::Type NativeFunction::long_long_type = &ffi_type_sint64;
    const NativeFunction::Type NativeFunction::float_type = &ffi_type_float;
    const NativeFunction::Type NativeFunction::double_type = &ffi_type_double;
    // Since strings (char*) are the only pointer types we support, we assume
    // pointer == string. If we supported other pointer types, we'd need to
    // introduce an enum to differentiate between types rather than using
    // ffi_types directly.
    const NativeFunction::Type NativeFunction::string_type = &ffi_type_pointer;

    NativeFunction::NativeFunction(const std::string& library,
                                   const std::string& name,
                                   Type return_type,
                                   std::vector<Type>& argument_types)
        : library(library), name(name), return_type(return_type), argument_types(argument_types)
    {
        if(library.size() == 0) library_handle = dlopen(nullptr, RTLD_LAZY);
        else library_handle = dlopen(library.c_str(), RTLD_LAZY);
        if(library_handle == nullptr) {
            util::error("Error opening library ", library);
        }
        function_handle = dlsym(library_handle, name.c_str());
        if(ffi_prep_cif(&cif,
                        FFI_DEFAULT_ABI,
                        this->argument_types.size(),
                        return_type,
                        this->argument_types.data()
                       ) != FFI_OK) {
            util::error("Error while initializing FFI for the declaration of ", name);
        }
    }

    std::shared_ptr<Value> to_value(short i) {
        return std::make_shared<IntValue>(i);
    }

    std::shared_ptr<Value> to_value(int i) {
        return std::make_shared<IntValue>(i);
    }

    std::shared_ptr<Value> to_value(long i) {
        return std::make_shared<IntValue>(i);
    }

    std::shared_ptr<Value> to_value(long long i) {
        return std::make_shared<IntValue>(i);
    }

    std::shared_ptr<Value> to_value(float f) {
        return std::make_shared<FloatValue>(f);
    }

    std::shared_ptr<Value> to_value(double f) {
        return std::make_shared<FloatValue>(f);
    }

    std::shared_ptr<Value> to_value(char* s) {
        return std::make_shared<StringValue>(s);
    }

    template<typename T>
    std::shared_ptr<Value> call_function(ffi_cif *cif, void *f, void **arguments) {
        if(sizeof(T) < sizeof(long)) {
            ffi_sarg result;
            ffi_call(cif, FFI_FN(f), &result, arguments);
            return to_value(T(result));
        } else {
            T result;
            ffi_call(cif, FFI_FN(f), &result, arguments);
            return to_value(result);
        }
    }

    [[noreturn]] void wrong_number_of_arguments(const std::string& name, int expected, int actual) {
        util::error("Wrong number of arguments to ", name,
                    ";  expected: ", expected,
                    ", but got: ", actual);
    }

    std::shared_ptr<Value> NativeFunction::call(const std::vector<std::shared_ptr<Value>>& arguments,
                                                Environment&) {
        if(arguments.size() != argument_types.size()) {
            wrong_number_of_arguments(name, argument_types.size(), arguments.size());
        }
        void** cargs = new void*[arguments.size()];
        for(size_t i = 0; i < arguments.size(); i++) {
            if(argument_types[i] == short_type) {
                cargs[i] = new short(arguments[i]->to_short());
            } else if(argument_types[i] == int_type) {
                cargs[i] = new int(arguments[i]->to_int());
            } else if(argument_types[i] == long_type) {
                cargs[i] = new long(arguments[i]->to_long());
            } else if(argument_types[i] == long_long_type) {
                cargs[i] = new long long(arguments[i]->to_long_long());
            } else if(argument_types[i] == float_type) {
                cargs[i] = new float(arguments[i]->to_float());
            } else if(argument_types[i] == double_type) {
                cargs[i] = new double(arguments[i]->to_double());
            } else if(argument_types[i] == string_type) {
                std::string str = arguments[i]->to_string();
                char** cstr = new char*;
                *cstr = new char[str.size() + 1];
                std::strcpy(*cstr, str.c_str());
                cargs[i] = cstr;
            } else {
                util::error("Unsupported argument type in declaration of native function, ", name);
            }
        }

        std::shared_ptr<Value> result;

        if(return_type == void_type) {
            ffi_call(&cif, FFI_FN(function_handle), nullptr, cargs);
            // Make void functions return 0 because we don't have a void type in FiffiScript
            result = std::make_shared<IntValue>(0);
        } else if(return_type == short_type) {
            result = call_function<short>(&cif, function_handle, cargs);
        } else if(return_type == int_type) {
            result = call_function<int>(&cif, function_handle, cargs);
        } else if(return_type == long_type) {
            result = call_function<long>(&cif, function_handle, cargs);
        } else if(return_type == long_long_type) {
            result = call_function<long long>(&cif, function_handle, cargs);
        } else if(return_type == float_type) {
            result = call_function<float>(&cif, function_handle, cargs);
        } else if(return_type == double_type) {
            result = call_function<double>(&cif, function_handle, cargs);
        } else if(return_type == string_type) {
            result = call_function<char*>(&cif, function_handle, cargs);
        } else {
            util::error("Unsupported return type in declaration of native function, ", name);
        }

        for(size_t i = 0; i < arguments.size(); i++) {
            if(argument_types[i] == short_type) {
                delete (short*)cargs[i];
            } else if(argument_types[i] == int_type) {
                delete (int*)cargs[i];
            } else if(argument_types[i] == long_type) {
                delete (long*)cargs[i];
            } else if(argument_types[i] == long_long_type) {
                delete (long long*)cargs[i];
            } else if(argument_types[i] == float_type) {
                delete (float*)cargs[i];
            } else if(argument_types[i] == double_type) {
                delete (double*)cargs[i];
            } else if(argument_types[i] == string_type) {
                delete[] (char*)(*((char**)cargs[i]));
                delete (char**)cargs[i];
            }
        }
        delete[] cargs;

        return result;
    }

    NativeFunction::~NativeFunction() {
        dlclose(library_handle);
    }

    std::shared_ptr<Value> Variable::evaluate(Environment& environment) {
        if(environment[name]) {
            return environment[name];
        } else {
            util::error("Undefined function or variable: ", name);
        }
    }

    std::shared_ptr<Value> FunctionCall::evaluate(Environment& environment) {
        std::shared_ptr<Value> function_value = function->evaluate(environment);
        std::vector<std::shared_ptr<Value>> argument_values(arguments.size());
        for(size_t i=0; i < arguments.size(); i++) {
            argument_values[i] = arguments[i]->evaluate(environment);
        }
        return function_value->call(argument_values, environment);
    }

    std::shared_ptr<Value> RegularFunction::call(const std::vector<std::shared_ptr<Value>>& arguments,
                                                 Environment& environment) {
        if(arguments.size() != parameters.size()) {
            wrong_number_of_arguments(name, parameters.size(), arguments.size());
        }
        environment.push_scope();
        for(size_t i = 0; i < arguments.size(); i++) {
            environment[parameters[i]] = arguments[i];
        }
        // Empty-bodied functions return 0 as we do not have a void value in FiffiScript
        // Otherwise the result of the last expression is returned
        std::shared_ptr<Value> result;
        if(body.size() == 0) {
            result = std::make_shared<IntValue>(0);
        } else {
            for(size_t i = 0; i < body.size() - 1; i++) {
                body[i]->evaluate(environment);
            }
            result = (*body.rbegin())->evaluate(environment);
        }
        environment.pop_scope();
        return result;
    }


    void Program::run() {
        Environment environment;
        for(const auto& definition : definitions) {
            environment[definition.name] = definition.body->evaluate(environment);
        }
        if(environment["main"]) {
            std::vector<std::shared_ptr<fiffiscript::Value>> no_arguments;
            environment["main"]->call(no_arguments, environment);
        } else {
            util::error("Function main() not found");
        }
    }
}
