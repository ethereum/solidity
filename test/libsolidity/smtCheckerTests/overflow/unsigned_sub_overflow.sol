pragma experimental SMTChecker;

contract C  {
	function f(uint x, uint y) public pure returns (uint) {
		return x - y;
	}
}
// ----
// Warning 3944: (113-118): Underflow (resulting value less than 0) happens here
