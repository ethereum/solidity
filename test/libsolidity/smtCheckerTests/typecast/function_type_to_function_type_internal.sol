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
// Warning 2519: (128-159): This declaration shadows an existing declaration.
// Warning 6031: (214-218): Internal error: Expression undefined for SMT solver.
// Warning 6031: (222-226): Internal error: Expression undefined for SMT solver.
// Warning 7229: (238-244): Assertion checker does not yet implement the type function (uint256) returns (uint256) for comparisons
// Warning 6328: (207-227): CHC: Assertion violation happens here.\nCounterexample:\na = 0, b = 0\n\n\n\nTransaction trace:\nconstructor()\nState: a = 0, b = 0\ng()
// Warning 6328: (231-245): CHC: Assertion violation happens here.\nCounterexample:\na = 0, b = 0\n\n\n\nTransaction trace:\nconstructor()\nState: a = 0, b = 0\ng()
// Warning 5729: (214-218): BMC does not yet implement this type of function call.
// Warning 5729: (222-226): BMC does not yet implement this type of function call.
// Warning 7229: (238-244): Assertion checker does not yet implement the type function (uint256) returns (uint256) for comparisons
// Warning 5729: (214-218): BMC does not yet implement this type of function call.
// Warning 5729: (222-226): BMC does not yet implement this type of function call.
// Warning 7229: (238-244): Assertion checker does not yet implement the type function (uint256) returns (uint256) for comparisons
