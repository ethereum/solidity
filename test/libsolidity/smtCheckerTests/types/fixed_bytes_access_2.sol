pragma experimental SMTChecker;
contract C {
	function f(bytes calldata x, uint y) external pure {
		x[8][0];
		x[8][5%y];
	}
}
// ----
// Warning 4281: (117-120): CHC: Division by zero happens here.\nCounterexample:\n\nx = [17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 20, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17]\ny = 0\n\n\nTransaction trace:\nconstructor()\nf([17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 20, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17], 0)
