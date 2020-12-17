pragma experimental SMTChecker;

contract C {
	uint x = 5;

	constructor(uint a, uint b) {
		assert(x == 5);
		x = a + b;
	}

	function f(uint y) view public {
		assert(y == x);
	}
}
// ----
// Warning 6328: (162-176): CHC: Assertion violation happens here.\nCounterexample:\nx = 0\ny = 1\n\n\nTransaction trace:\nconstructor(0, 0)\nState: x = 0\nf(1)
// Warning 4984: (115-120): CHC: Overflow (resulting value larger than 2**256 - 1) happens here.\nCounterexample:\nx = 5\na = 115792089237316195423570985008687907853269984665640564039457584007913129639935\nb = 1\n\n\nTransaction trace:\nconstructor(115792089237316195423570985008687907853269984665640564039457584007913129639935, 1)
