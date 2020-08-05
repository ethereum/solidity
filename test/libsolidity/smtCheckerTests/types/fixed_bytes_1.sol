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
// ----
// Warning 6328: (96-110): Assertion violation happens here
// Warning 6328: (114-130): Assertion violation happens here
