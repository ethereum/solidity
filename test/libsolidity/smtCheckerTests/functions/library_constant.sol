pragma experimental SMTChecker;

library l1 {

	uint private constant TON = 1000;
	function f1() public pure {
		assert(TON == 1000);
		assert(TON == 2000);
	}
	function f2(uint x, uint y) internal pure returns (uint) {
		return x + y;
	}
}

contract C {
	function f(uint x) public pure {
		uint z = l1.f2(x, 1);
		assert(z == x + 1);
	}
}
// ----
// Warning 6328: (136-155): CHC: Assertion violation happens here.\nCounterexample:\nTON = 1000\n\n\n\nTransaction trace:\nconstructor()\nState: TON = 1000\nf1()
// Warning 4984: (229-234): CHC: Overflow (resulting value larger than 2**256 - 1) happens here.\nCounterexample:\n\nx = 115792089237316195423570985008687907853269984665640564039457584007913129639935\n\n\nTransaction trace:\nconstructor()\nf(115792089237316195423570985008687907853269984665640564039457584007913129639935)
// Warning 4984: (327-332): CHC: Overflow (resulting value larger than 2**256 - 1) happens here.\nCounterexample:\n\nx = 115792089237316195423570985008687907853269984665640564039457584007913129639935\n\n\nTransaction trace:\nconstructor()\nf(115792089237316195423570985008687907853269984665640564039457584007913129639935)
