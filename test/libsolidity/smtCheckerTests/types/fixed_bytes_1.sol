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
// Warning 6328: (96-110): CHC: Assertion violation happens here.\nCounterexample:\nx = 0\ny = 0\n\n\nTransaction trace:\nconstructor()\nState: x = 0\nf(0)
// Warning 6328: (114-130): CHC: Assertion violation happens here.\nCounterexample:\nx = 0\ny = 0\n\n\nTransaction trace:\nconstructor()\nState: x = 0\nf(0)
