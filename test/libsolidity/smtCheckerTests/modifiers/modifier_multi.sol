contract C
{
	uint x;

	modifier m {
		require(x > 0);
		require(x < 10000);
		_;
	}

	modifier n {
		x = x + 1;
		_;
		assert(x > 2);
		assert(x > 8);
	}

	function f() m n public {
		x = x + 1;
	}

	function g(uint _x) public {
		x = _x;
	}
}
// ====
// SMTEngine: all
// ----
// Warning 6328: (137-150): CHC: Assertion violation happens here.
// Info 1391: CHC: 3 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
