type Int is int16;

using {add as +} for Int global;

function add(Int, Int) pure returns (Int) {}

function f() {
    Int a;
    a.+(a);
}
// ----
// ParserError 2314: (132-133): Expected identifier but got '+'
