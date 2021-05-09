contract C {
	function g() internal pure returns (uint, uint) {
		return (2, 3);
	}
	function f() public {
		(address(1).call(""));
		(uint x, uint y) = ((g()));
		assert(x == 2);
		assert(y == 3);
	}
}
// ====
// SMTEngine: all
// ----
// Warning 4588: (110-129): Assertion checker does not yet implement this type of function call.
// Warning 4588: (110-129): Assertion checker does not yet implement this type of function call.
