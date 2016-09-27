#ifndef UTIL_HH
#define UTIL_HH

#include <iostream>
#include <string>
#include <cstdlib>

#include "location.hh"

// This is a class with only static members rather than a namespace, so I can
// define private helper functions
class util {
    // Print the given arguments to cerr
    template<typename Head, typename ...Tail>
    static void print(const Head& head, const Tail&... tail)
    {
        std::cerr << head;
        print(tail...);
    }

    static void print() {
    }

    template<typename ...T>
    [[noreturn]] static void println_and_exit(const T&... args)
    {
        print(args...);
        std::cerr << std::endl;
        std::exit(1);
    }

public:
    template<typename ...T>
    [[noreturn]] static void error(const yy::location& loc, const T&... args)
    {
        println_and_exit(loc, ": ", args...);
    }

    template<typename ...T>
    [[noreturn]] static void error(const T&... args)
    {
        println_and_exit(args...);
    }
};
#endif
