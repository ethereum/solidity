pragma experimental SMTChecker;
contract C {
	function f(bytes calldata x, uint y) external pure {
		x[8][0];
		x[8][5%y];
	}
}
// ----
// Warning 3046: (117-120): BMC: Division by zero happens here.
