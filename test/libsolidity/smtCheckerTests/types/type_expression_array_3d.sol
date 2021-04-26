contract C {

function f() public pure { int[][][]; }

}
// ====
// SMTEngine: all
// ----
// Warning 6133: (41-50): Statement has no effect.
// Warning 8364: (41-46): Assertion checker does not yet implement type type(int256[] memory)
// Warning 8364: (41-48): Assertion checker does not yet implement type type(int256[] memory[] memory)
// Warning 8364: (41-50): Assertion checker does not yet implement type type(int256[] memory[] memory[] memory)
// Warning 8364: (41-46): Assertion checker does not yet implement type type(int256[] memory)
// Warning 8364: (41-48): Assertion checker does not yet implement type type(int256[] memory[] memory)
// Warning 8364: (41-50): Assertion checker does not yet implement type type(int256[] memory[] memory[] memory)
