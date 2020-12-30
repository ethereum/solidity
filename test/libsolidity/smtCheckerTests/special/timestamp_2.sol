pragma experimental SMTChecker;

contract C {
	uint x;

	constructor() {
		x = block.timestamp + 0; // No overflow should be reported
		x = block.timestamp + 1; // Overflow should be reported here
	}
}
// ----
// Warning 4984: (140-159): CHC: Overflow (resulting value larger than 2**256 - 1) happens here.\nCounterexample:\nx = 115792089237316195423570985008687907853269984665640564039457584007913129639935\n\nTransaction trace:\nC.constructor()
