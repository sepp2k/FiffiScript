# FiffiScript
## Overview

This is an interpreter for FiffiScript.

FiffiScript is a small, decidedly non-Turing complete, programming language
whose only notable ability is to declare and call native functions. This
project primarily exists as an example of how one might interface with native
functions in a programming language interpreter.

This project was quickly thrown together to demonstrate how to use dlopen and
libffi to create a working FFI in an interpreter. It should not be considered
an example of good practices in interpreter design (for example it has horrible
error reporting, does not keep track of location information, executes the code
using a simple AST traversal, does not include "advanced" features like
arithmetic or loops, is under-documented and under-tested and is generally just
the result of trying to get something to work quickly).

## Dependencies

### Build Dependencies

* flex
* bison
* make
* gcc/g++
* libffi

### Runtime Dependencies

* libffi

## Features

FiffiScript supports 64-bit integer and floating point values as well as
strings.

It supports calling fixed-arity native functions that take arguments
of the C types `short`, `int`, `long`, `long long`, `float`, `double`and `char*`
and have a return type that's either one of those or `void`.
Native functions can either be defined in the C standard library or in an
external library (whose name you'd specify when declaring the function).

When calling native functions, it can automatically convert between numeric
types and from numeric types to string.
Since there is no `void` type in FiffiScript itself, calling a `void` native
function will return the integer value `0` instead.

You can also define FiffiScript functions. A FiffiScript function has a
number of typeless parameters and a body, which is a sequence of expressions.
The return value of a function is the value of the last expression in the
function body. If the body is empty, the return value is `0`.

An expression is either a constant expression (integer, floating point or
string literal), a variable or a function call.

No arithmetic operators are built in.

A variable is the name of a global definition (this includes function
definitions, so you can pass function references around) or a parameter
of the current function.
There are presently no local variable definitions.

Comments begin with an `#` sign and extend to the end of the line.

## Usage

Build using `make` and then run `./fiffiscript foo.fiffi` where `foo.fiffi` is
the file containing your FiffiScript code.

You can also invoke it without arguments, in which case it will read the code
from stdin.

## Examples

Examples can be found in the examples directory.
`examples/external_library.fiffi` is the one that contains actual comments, so
you should look at that.

## Code Organization

The syntax of the language is implemented in tokenizer.l and parser.yy.
Its semantics are implemented in fiffiscript.{cc,hh}.
In particular the code implementing the FFI lives in the class `NativeFunction`.

## License

As stated above, this is meant as an example of how to implement an FFI. Feel
free to use any parts of the code in whatever fashion you see fit.
