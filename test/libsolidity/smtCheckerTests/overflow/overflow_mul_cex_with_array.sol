pragma experimental SMTChecker;

contract C {
	function f(bytes calldata x, uint y) external pure {
		x[8][0];
		x[8][5*y];
	}
}
// ----
// Warning 4984: (118-121): CHC: Overflow (resulting value larger than 2**256 - 1) happens here.
