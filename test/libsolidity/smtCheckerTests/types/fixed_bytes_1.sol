pragma experimental SMTChecker;

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
// SMTIgnoreCex: yes
// ----
// Warning 6328: (96-110): CHC: Assertion violation happens here.
// Warning 6328: (114-130): CHC: Assertion violation happens here.
