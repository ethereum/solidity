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
// Info 1391: CHC: 2 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
