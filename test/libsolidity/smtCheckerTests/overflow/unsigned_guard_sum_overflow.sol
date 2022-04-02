contract C  {
	function f(uint x, uint y) public pure returns (uint) {
		return x + y;
	}
}
// ====
// SMTEngine: all
// SMTIgnoreCex: yes
// ----
// Warning 4984: (80-85='x + y'): CHC: Overflow (resulting value larger than 2**256 - 1) happens here.
