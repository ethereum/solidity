contract C
{
	function f(uint x) internal pure returns (uint, bool, uint) {
		bool b = true;
		uint y = 999;
		return (x * 2, b, y);
	}
	function g() public pure {
		(uint x, bool b, uint y) = f(7);
		assert(x == 14);
		assert(b);
		assert(y == 999);
	}
}
// ====
// SMTEngine: all
// ----
// Info 1391: CHC: 4 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
