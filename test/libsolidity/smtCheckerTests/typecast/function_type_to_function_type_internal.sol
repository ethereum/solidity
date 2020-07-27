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
// Warning 6328: (207-227): Assertion violation happens here
// Warning 6328: (231-245): Assertion violation happens here
// Warning 5729: (214-218): Assertion checker does not yet implement this type of function call.
// Warning 5729: (222-226): Assertion checker does not yet implement this type of function call.
// Warning 7229: (238-244): Assertion checker does not yet implement the type function (uint256) returns (uint256) for comparisons
// Warning 5729: (214-218): Assertion checker does not yet implement this type of function call.
// Warning 5729: (222-226): Assertion checker does not yet implement this type of function call.
// Warning 7229: (238-244): Assertion checker does not yet implement the type function (uint256) returns (uint256) for comparisons
