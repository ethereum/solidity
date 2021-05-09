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
// ====
// SMTEngine: all
// SMTIgnoreCex: yes
// ----
// Warning 6328: (158-173): CHC: Assertion violation happens here.
