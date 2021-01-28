pragma experimental SMTChecker;
contract C {
	function f(bytes calldata x, uint y) external pure {
		x[8][0];
		x[8][5%y];
	}
}
// ----
// Warning 4281: (117-120): CHC: Division by zero happens here.\nCounterexample:\n\ny = 0\n\nTransaction trace:\nC.constructor()\nC.f(x, 0)
