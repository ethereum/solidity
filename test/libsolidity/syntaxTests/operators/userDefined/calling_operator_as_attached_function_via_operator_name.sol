type Int is int16;

using {add as +} for Int;

function add(Int, Int) pure returns (Int) {}

function f() {
    Int a;
    a.+(a);
}
// ----
// ParserError 2314: (125-126): Expected identifier but got '+'
