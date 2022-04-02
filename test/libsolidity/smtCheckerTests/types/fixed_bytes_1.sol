contract C
{
	bytes32 x;
	function f(bytes8 y) public view {
		assert(x != y);
		assert(x != g());
	}
	function g() public view returns (bytes32) {
		return x;
	}
}
// ====
// SMTEngine: all
// SMTIgnoreCex: yes
// ----
// Warning 6328: (63-77='assert(x != y)'): CHC: Assertion violation happens here.
// Warning 6328: (81-97='assert(x != g())'): CHC: Assertion violation happens here.
