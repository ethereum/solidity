pragma experimental SMTChecker;
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
// ----
// Warning 4588: (142-161): Assertion checker does not yet implement this type of function call.
// Warning 4588: (142-161): Assertion checker does not yet implement this type of function call.
