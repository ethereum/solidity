abstract contract D
{
	function g(uint x) public virtual;
}

contract C
{
	uint x;
	function f(uint y, D d) public {
		require(x == y);
		assert(x == y);
		d.g(y);
		// Storage knowledge is cleared after an external call.
		assert(x == y);
	}
}
// ====
// SMTEngine: all
// ----
// Warning 0: (61-244): Contract invariants for :C:\n!(<errorCode> = 1)\n((!(x <= 0) || !(<errorCode> >= 2)) && (!(x <= 0) || (x' <= 0)))\n(x <= 0)\n
