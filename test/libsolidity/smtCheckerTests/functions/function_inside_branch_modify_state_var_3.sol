pragma experimental SMTChecker;

contract C
{
	uint x;
	function f() internal {
		require(x < 10000);
		x = x + 1;
	}
	function g(bool b) public {
		x = 0;
		if (b)
			f();
		// Should fail for `b == true`.
		assert(x == 0);
	}
	function h(bool b) public {
		x = 0;
		if (!b)
			f();
		// Should fail for `b == false`.
		assert(x == 0);
	}

}
// ----
// Warning 6328: (209-223): CHC: Assertion violation happens here.\nCounterexample:\nx = 1\nb = true\n\n\nTransaction trace:\nconstructor()\nState: x = 0\ng(true)
// Warning 6328: (321-335): CHC: Assertion violation happens here.\nCounterexample:\nx = 1\nb = false\n\n\nTransaction trace:\nconstructor()\nState: x = 0\nh(false)
