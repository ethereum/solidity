pragma experimental SMTChecker;
contract C {
    function f(function(uint) returns (uint) g, function(uint) returns (uint) h) internal {
		assert(g(2) == h(2));
		assert(g == h);
    }
}
// ----
// Warning: (146-150): Assertion checker does not yet implement this type of function call.
// Warning: (154-158): Assertion checker does not yet implement this type of function call.
// Warning: (170-176): Assertion checker does not yet implement the type function (uint256) returns (uint256) for comparisons
// Warning: (139-159): Assertion violation happens here
// Warning: (163-177): Assertion violation happens here
