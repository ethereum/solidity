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
		unchecked { x = x + 1; }
	}

	function g(uint _x) public {
		x = _x;
	}
}
// ----
// Warning 6328: (136-149): CHC: Assertion violation happens here.\nCounterexample:\nx = 0\n\nTransaction trace:\nC.constructor()\nState: x = 0\nC.g(115792089237316195423570985008687907853269984665640564039457584007913129639935)\nState: x = 115792089237316195423570985008687907853269984665640564039457584007913129639935\nC.f()
