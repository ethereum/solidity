pragma experimental SMTChecker;

contract C {
    struct A { uint a; uint b; }
    function f() public pure returns (uint) {
        A memory a = A({ a: 1, b: 2 });
    }
}
// ----
// Warning 2072: (133-143): Unused local variable.
// Warning 8115: (133-143): Assertion checker does not yet support the type of this variable.
// Warning 8364: (146-147): Assertion checker does not yet implement type type(struct C.A storage pointer)
// Warning 8364: (146-163): Assertion checker does not yet implement type struct C.A memory
// Warning 4639: (146-163): Assertion checker does not yet implement this expression.
