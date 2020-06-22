pragma experimental SMTChecker;
contract C {

function f() public pure { int[][]; }

}
// ----
// Warning 6133: (73-80): Statement has no effect.
// Warning 8364: (73-78): Assertion checker does not yet implement type type(int256[] memory)
// Warning 8364: (73-80): Assertion checker does not yet implement type type(int256[] memory[] memory)
