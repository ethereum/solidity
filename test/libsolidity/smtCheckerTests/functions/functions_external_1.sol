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
// SMTIgnoreOS: macos
// ----
// Info 1180: Contract invariant(s) for :C:\n(x <= 0)\nReentrancy property(ies) for :C:\n!(<errorCode> = 1)\n((!(x <= 0) || !(<errorCode> >= 2)) && (!(x <= 0) || (x' <= 0)))\n<errorCode> = 0 -> no errors\n<errorCode> = 1 -> Assertion failed at assert(x == y)\n<errorCode> = 2 -> Assertion failed at assert(x == y)\n
