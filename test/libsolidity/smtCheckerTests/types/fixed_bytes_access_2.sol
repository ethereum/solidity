pragma experimental SMTChecker;
contract C {
	function f(bytes calldata x, uint y) external pure {
		x[8][0];
		x[8][5%y];
	}
}
// ----
// Warning 7989: (101-108): Assertion checker does not yet support index accessing fixed bytes.
// Warning 7989: (112-121): Assertion checker does not yet support index accessing fixed bytes.
// Warning 3046: (117-120): Division by zero happens here.
