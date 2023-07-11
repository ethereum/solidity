function f(uint) pure returns (uint) {
	return 2;
}
function f(string memory) pure returns (uint) {
	return 3;
}

contract C {
	function g() public pure {
		(uint x, uint y) = (f(2), f("abc"));
		assert(x == 2); // should hold
		assert(y == 4); // should fail
	}
}
// ====
// SMTEngine: all
// ----
// Warning 6328: (229-243): CHC: Assertion violation happens here.
// Info 1391: CHC: 1 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
