pragma experimental SMTChecker;

contract C
{
	uint x;

	modifier m {
		require(x > 0);
		require(x < 10000);
		_;
		assert(x > 1);
		_;
		assert(x > 2);
		assert(x > 10);
	}

	function f() m public {
		x = x + 1;
	}

	function g(uint _x) public {
		x = _x;
	}
}
// ----
// Warning 6328: (156-170): CHC: Assertion violation happens here.\nCounterexample:\nx = 3\n\n\n\nTransaction trace:\nconstructor()\nState: x = 0\ng(1)\nState: x = 1\nf()
