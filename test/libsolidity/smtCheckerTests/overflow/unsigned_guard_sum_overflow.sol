pragma experimental SMTChecker;

contract C  {
	function f(uint x, uint y) public pure returns (uint) {
		return x + y;
	}
}
// ====
// SMTIgnoreCex: yes
// ----
// Warning 4984: (113-118): CHC: Overflow (resulting value larger than 2**256 - 1) happens here.
