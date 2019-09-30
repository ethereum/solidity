pragma experimental SMTChecker;
contract C {
	function(uint) returns (uint) a;
	function(uint) returns (uint) b;
    function f(function(uint) returns (uint) g, function(uint) returns (uint) h) internal {
		assert(g(2) == h(2));
		assert(g == h);
    }
	function g() public {
		f(a, b);
	}
}
// ----
// Warning: (214-218): Assertion checker does not yet implement this type of function call.
// Warning: (222-226): Assertion checker does not yet implement this type of function call.
// Warning: (238-244): Assertion checker does not yet implement the type function (uint256) returns (uint256) for comparisons
// Warning: (207-227): Assertion violation happens here
// Warning: (214-218): Assertion checker does not yet implement this type of function call.
// Warning: (222-226): Assertion checker does not yet implement this type of function call.
// Warning: (238-244): Assertion checker does not yet implement the type function (uint256) returns (uint256) for comparisons
// Warning: (207-227): Assertion violation happens here
