pragma experimental SMTChecker;

contract C
{
	uint x;

	modifier m {
		require(x > 0);
		_;
		// Fails because of overflow behavior.
		assert(x > 1);
	}

	function f() m public {
		assert(x > 0);
		x = x + 1;
	}

	function g(uint _x) public {
		x = _x;
	}
}
// ----
// Warning 4984: (203-208): CHC: Overflow (resulting value larger than 2**256 - 1) happens here.\nCounterexample:\nx = 115792089237316195423570985008687907853269984665640564039457584007913129639935\n\n\n\nTransaction trace:\nconstructor()\nState: x = 0\ng(115792089237316195423570985008687907853269984665640564039457584007913129639935)\nState: x = 115792089237316195423570985008687907853269984665640564039457584007913129639935\nf()
// Warning 6328: (136-149): CHC: Assertion violation happens here.\nCounterexample:\nx = 0\n\n\n\nTransaction trace:\nconstructor()\nState: x = 0\ng(115792089237316195423570985008687907853269984665640564039457584007913129639935)\nState: x = 115792089237316195423570985008687907853269984665640564039457584007913129639935\nf()
