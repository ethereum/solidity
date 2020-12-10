pragma experimental SMTChecker;

contract C
{
	function f(uint _x) public pure returns (uint) {
		return _x;
	}
}

contract D
{
	C c;
	function g(uint _y) public view {
		uint z = c.f(_y);
		assert(z == _y);
	}
}
// ----
// Warning 6328: (191-206): CHC: Assertion violation happens here.\nCounterexample:\nc = 0\n_y = 0\n\n\nTransaction trace:\nconstructor()\nState: c = 0\ng(0)
