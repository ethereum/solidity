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
// Warning: (180-187): Internal error: Expression undefined for SMT solver.
// Warning: (191-206): Assertion violation happens here
