contract C
{
	bytes32 x;
	function f(bytes8 y) public view {
		assert(x == g());
		assert(x != y);
	}
	function g() public view returns (bytes32) {
		return x;
	}
}
// ====
// SMTEngine: all
// ----
// Warning 6328: (83-97): CHC: Assertion violation happens here.
// Info 1391: CHC: 1 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
