pragma experimental SMTChecker;

contract C {
    struct A { uint a; uint b; }
    function f() public pure returns (uint) {
        A memory a = A({ a: 1, b: 2 });
    }
}
// ----
// Warning: (133-143): Unused local variable.
// Warning: (133-143): Assertion checker does not yet support the type of this variable.
// Warning: (146-163): Assertion checker does not yet implement type struct C.A memory
// Warning: (146-163): Assertion checker does not yet implement this expression.
