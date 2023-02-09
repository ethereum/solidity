contract C
{
	uint x;
	function f(uint y) public returns (uint) {
		x = y;
		return x;
	}
	function g(uint y) public {
		require(y < 1000);
		uint z = this.f(y);
		assert(z < 1000);
	}
}
// ====
// SMTEngine: all
// ----
// Info 1391: CHC: 1 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
