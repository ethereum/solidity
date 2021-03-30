pragma experimental SMTChecker;

contract C {
	function r(bytes32 x, uint y) public pure {
		x[0]; // safe access
		x[y]; // oob access
	}
}
// ----
// Warning 6368: (116-120): CHC: Out of bounds access happens here.
