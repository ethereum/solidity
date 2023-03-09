library L {
	function l(uint x, uint y) internal pure returns (uint) {
		return x + y;
	}
}

contract C {
	function f(uint u, uint s, bool b) internal pure returns (uint z) {
		if (b)
			z = u;
		else
			z = s;
	}

	using L for uint;

	function call() public pure {
		uint a = 2;
		uint b = a.l({y: 3});
		assert(b == 5);
		b = L.l({x: 3, y: 3});
		assert(b == 6);
		b = f({b: true, u: 1, s: 2});
		assert(b == 1);
		b = f({b: false, u: 1, s: 2});
		// Fails, should be 2.
		assert(b == 6);
	}
}
// ====
// SMTEngine: all
// ----
// Warning 6328: (475-489): CHC: Assertion violation happens here.
// Info 1391: CHC: 4 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
