pragma experimental SMTChecker;

contract C {
	function f(bytes calldata x, uint y) external pure {
		require(x.length > 10);
		x[8][0];
		x[8][5*y];
	}
}
// ====
// SMTIgnoreCex: yes
// ----
// Warning 4984: (144-147): CHC: Overflow (resulting value larger than 2**256 - 1) happens here.
// Warning 6368: (139-148): CHC: Out of bounds access happens here.
