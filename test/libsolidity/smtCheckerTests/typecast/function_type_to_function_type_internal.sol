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
// Warning 2519: (96-127='function(uint) returns (uint) g'): This declaration shadows an existing declaration.
// Warning 6031: (182-186='g(2)'): Internal error: Expression undefined for SMT solver.
// Warning 6031: (190-194='h(2)'): Internal error: Expression undefined for SMT solver.
// Warning 7229: (206-212='g == h'): Assertion checker does not yet implement the type function (uint256) returns (uint256) for comparisons
// Warning 6328: (175-195='assert(g(2) == h(2))'): CHC: Assertion violation happens here.\nCounterexample:\na = 0, b = 0\n\nTransaction trace:\nC.constructor()\nState: a = 0, b = 0\nC.g()\n    C.f(0, 0) -- internal call
// Warning 6328: (199-213='assert(g == h)'): CHC: Assertion violation happens here.\nCounterexample:\na = 0, b = 0\n\nTransaction trace:\nC.constructor()\nState: a = 0, b = 0\nC.g()\n    C.f(0, 0) -- internal call
// Warning 5729: (182-186='g(2)'): BMC does not yet implement this type of function call.
// Warning 5729: (190-194='h(2)'): BMC does not yet implement this type of function call.
