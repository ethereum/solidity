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
// ====
// SMTEngine: all
// ----
// Warning 2519: (96-127): This declaration shadows an existing declaration.
// Warning 6031: (182-186): Internal error: Expression undefined for SMT solver.
// Warning 6031: (190-194): Internal error: Expression undefined for SMT solver.
// Warning 7229: (206-212): Assertion checker does not yet implement the type function (uint256) returns (uint256) for comparisons
// Warning 5729: (182-186): BMC does not yet implement this type of function call.
// Warning 5729: (190-194): BMC does not yet implement this type of function call.
// Warning 6328: (175-195): CHC: Assertion violation happens here.
// Warning 6328: (199-213): CHC: Assertion violation happens here.
