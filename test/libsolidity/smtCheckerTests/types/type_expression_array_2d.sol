pragma experimental SMTChecker;
contract C {

function f() public pure { int[][]; }

}
// ----
// Warning: (73-80): Statement has no effect.
// Warning: (73-78): Assertion checker does not yet implement type type(int256[] memory)
// Warning: (73-78): Assertion checker does not yet implement this expression.
// Warning: (73-80): Assertion checker does not yet implement type type(int256[] memory[] memory)
