pragma experimental SMTChecker;

contract C {
    struct A { uint a; uint b; }
    function f() public pure returns (uint) {
        A memory a = A({ a: 1, b: 2 });
    }
}
// ----
// Warning 6321: (117-121): Unnamed return variable can remain unassigned. Add an explicit return with value to all non-reverting code paths or name the variable.
// Warning 2072: (133-143): Unused local variable.
// Warning 8364: (146-147): Assertion checker does not yet implement type type(struct C.A storage pointer)
// Warning 4639: (146-163): Assertion checker does not yet implement this expression.
// Warning 8364: (146-147): Assertion checker does not yet implement type type(struct C.A storage pointer)
// Warning 4639: (146-163): Assertion checker does not yet implement this expression.
