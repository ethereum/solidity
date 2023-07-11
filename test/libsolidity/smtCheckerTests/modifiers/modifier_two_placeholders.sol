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
// ====
// SMTEngine: all
// ----
// Warning 6328: (123-137): CHC: Assertion violation happens here.
// Info 1391: CHC: 3 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
