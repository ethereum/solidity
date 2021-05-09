contract C {
	function f(bytes calldata x, uint y) external pure {
		require(x.length > 10);
		x[8][0];
		x[8][5*y];
	}
}
// ====
// SMTEngine: all
// SMTIgnoreCex: yes
// ----
// Warning 4984: (111-114): CHC: Overflow (resulting value larger than 2**256 - 1) happens here.
// Warning 6368: (106-115): CHC: Out of bounds access happens here.
